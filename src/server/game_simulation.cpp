#include "game_simulation.hpp"
#include "../config.hpp"
#include "../events.hpp"
#include "../shared/characters/character_movement.hpp"
#include "../shared/characters/character_types.hpp"
#include "../shared/map/grid.hpp"
#include "../shared/map/map_loader.hpp"
#include "../shared/state/player_state.hpp"
#include "raylib.h"
#include "systems/spawn_system.hpp"
#include <algorithm>
#include <cstdint>
#include <string.h>
#include <sys/types.h>

void GameSimulation::Initialize(EventBus &eventBus) {
    m_players = {};
    m_map = Map::LoadMap(ACTIVE_MAP);
    m_eventBus = &eventBus;
    m_abilitySystem.Initialize(eventBus, m_map);
    m_bulletSystem.SetMap(m_map);
    m_wallManager.Initialize(*m_eventBus);
    m_bulletSystem.Initialize(m_wallManager, m_abilitySystem, *m_eventBus);
}

void GameSimulation::Reset() {
    m_gameTime = MATCH_TIME;
    m_players.fill(state::PlayerState{});
    m_bulletSystem.Reset();
    m_wallManager.Reset();
}

void GameSimulation::Update(float tickRate) {
    std::vector<Collision::AABB> dynamicColliders = m_wallManager.GetColliders();
    for (auto &player : m_players) {
        const auto &def = GetCharacterDef(player.characterId);
        if (!player.active)
            continue;

        if (player.respawnTimer > 0.0f) {
            player.respawnTimer -= tickRate;
            if (player.respawnTimer <= 0.0f) {
                player.respawnTimer = 0.0f;
                RespawnPlayer(player);
            }
            continue; // skip movement/input while dead
        }

        if (player.invincibilityTimer > 0.0f)
            player.invincibilityTimer -= tickRate;

        if (player.shootTimer > 0.0f)
            player.shootTimer -= tickRate;

        if (player.wallTimer > 0.0f)
            player.wallTimer -= tickRate;

        // The regenTimer is init to 0.5 so that health regens right after 5 seconds istead of 5.5 seconds
        player.lastDamageTakenTimer += tickRate;
        if (player.lastDamageTakenTimer >= 3.0f) {
            player.healthRegenTimer += tickRate;
            if (player.healthRegenTimer >= 0.5f) {
                player.health = std::min(def.maxHealth, player.health + 2.0f);
                player.healthRegenTimer = 0.0f;
            }
        }

        SimulatePlayerInput(player, tickRate, dynamicColliders);
    }

    m_bulletSystem.Update(tickRate, m_players, m_wallManager.GetAllWalls());
    m_abilitySystem.Update(tickRate, m_players);
    m_gameTime -= tickRate;
}

void GameSimulation::SimulatePlayerInput(state::PlayerState &player, float tickRate,
                                         std::vector<Collision::AABB> &dynamicColliders) {
    player.prevButtons = player.lastButtons;
    player.lastButtons = player.currentInput.buttons;

    Vector2 spawnPos = player.position;
    SimulatePlayerMovement(player, tickRate, dynamicColliders);

    const Character::CharacterDef &charDef = Character::GetCharacterDef(player.characterId);
    SimulatePlayerShoot(player, charDef, spawnPos);
    SimulatePlayerWallPlacement(player, charDef);
}

void GameSimulation::SimulatePlayerMovement(state::PlayerState &player, float tickRate,
                                            std::vector<Collision::AABB> &dynamicColliders) {

    Vector2 moveInput = {player.currentInput.moveX, player.currentInput.moveY};
    bool isMoving = (moveInput.x * moveInput.x + moveInput.y * moveInput.y) > 0.0001f;

    if (isMoving) {
        player.state = state::State::Running;
    } else {
        player.state = state::State::Idle;
    }

    Character::SimulateMove(player, tickRate, m_map.walls, dynamicColliders);
}

void GameSimulation::SimulatePlayerShoot(state::PlayerState &player, Character::CharacterDef charDef,
                                         Vector2 spawnPos) {
    bool wantsToShoot = player.currentInput.buttons & (1 << 0);
    bool newShot = wantsToShoot && (player.currentInput.predBulletSequence != player.lastPredBulletSequence);

    if (newShot && player.shootTimer <= 0.0f) {
        player.lastPredBulletSequence = player.currentInput.predBulletSequence;
        Vector2 aimDir = {player.currentInput.aimX, player.currentInput.aimY};
        m_bulletSystem.Spawn({player.id, spawnPos, aimDir, charDef, player.currentInput.predBulletSequence});
        player.shootTimer = charDef.bullet.cooldown;
    }
}

void GameSimulation::SimulatePlayerWallPlacement(state::PlayerState &player, Character::CharacterDef charDef) {
    bool placeNow = player.currentInput.buttons & (1 << 1);
    bool placePrev = player.prevButtons & (1 << 1);

    // Picking up and placing walls are on the same wall timer
    if (placeNow && !placePrev && player.wallTimer <= 0.0f) {
        HandleWallInput(player, player.currentInput, charDef);
    }
}

void GameSimulation::RespawnPlayer(state::PlayerState &player) {
    const auto &def = GetCharacterDef(player.characterId);
    player.state = state::State::Idle;
    player.health = def.maxHealth;
    player.position = System::Spawn(m_map.spawnPoints, m_players);
    player.invincibilityTimer = 2.0f;
    player.velocity = {0, 0};
    player.respawnTimer = 0.0f;
    player.shootTimer = 0.0f;
    player.lastDamageTakenTimer = 0.0f;
    player.healthRegenTimer = 0.5f;
    m_eventBus->publish(event::PlayerRespawnEvent{player});
}

const std::array<state::BulletState, MAX_BULLETS> &GameSimulation::GetBullets() { return m_bulletSystem.GetBullets(); }

System::BulletSystem<state::BulletState> &GameSimulation::GetBulletSystem() { return m_bulletSystem; }

Map::WallManager &GameSimulation::GetWallManager() { return m_wallManager; }

const Map::WallManager &GameSimulation::GetWallManager() const { return m_wallManager; }

void GameSimulation::ApplyInput(uint32_t playerId, Character::CharacterId characterId,
                                const state::PlayerInput &input) {
    if (playerId > MAX_PLAYERS)
        return;

    state::PlayerState &player = m_players[playerId];
    if (!player.active || player.respawnTimer > 0.0f)
        return;

    player.currentInput = input;
}

void GameSimulation::HandleWallInput(state::PlayerState &player, const state::PlayerInput &input,
                                     const Character::CharacterDef &charDef) {
    const Map::Vector2i gridPos = Map::WorldToGrid({input.aimX, input.aimY});
    if (TryPlaceWall(player, gridPos)) {
        player.wallTimer = charDef.wallCooldown;
        return;
    }

    if (m_wallManager.PickUpWall(gridPos, player.id)) {
        player.wallTimer = charDef.wallCooldown;
    }
}

bool GameSimulation::TryPlaceWall(state::PlayerState &player, Map::Vector2i gridPos) {
    if (!m_wallManager.CanPlaceWall(gridPos, m_map.walls, player, m_players))
        return false;

    player.currentAvaliableWalls--;
    m_wallManager.PlaceWall(gridPos, 50, player);
    return true;
}

const std::array<state::PlayerState, MAX_PLAYERS> &GameSimulation::GetPlayers() const { return m_players; }

void GameSimulation::CreatePlayer(uint32_t playerId, Character::CharacterId characterId, const char *name) {
    if (playerId > MAX_PLAYERS)
        return;
    const Character::CharacterDef &charDef = GetCharacterDef(characterId);
    Vector2 spawn = System::InitSpawn(m_map.spawnPoints, playerId);
    const auto &def = GetCharacterDef(characterId);
    state::PlayerState player = {.id = playerId,
                                 .characterId = characterId,
                                 .position = spawn,
                                 .health = def.maxHealth,
                                 .hurtbox = {.radius = charDef.hurtboxRadius},
                                 .lastButtons = 0,
                                 .currentAvaliableWalls = def.maxWalls,
                                 .active = true};
    strncpy(player.name, name, sizeof(player.name) - 1);
    player.name[sizeof(player.name) - 1] = '\0';
    m_players[playerId] = player;
}

void GameSimulation::RemovePlayer(uint32_t playerId) {
    if (playerId >= MAX_PLAYERS)
        return;
    state::PlayerState &player = m_players[playerId];
    player.active = false;
}

float GameSimulation::GetGameTime() const { return m_gameTime; }
