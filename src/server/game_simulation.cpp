#include "game_simulation.hpp"
#include "../config.hpp"
#include "../shared/characters/character_movement.hpp"
#include "../shared/map/grid.hpp"
#include "../shared/map/map_loader.hpp"
#include "characters/character_types.hpp"
#include "events.hpp"
#include "raylib.h"
#include "state/player_state.hpp"
#include "systems/spawn_system.hpp"
#include <cstdint>
#include <string.h>
#include <sys/types.h>

void GameSimulation::Initialize(EventBus &eventBus) {
    m_players = {};
    m_map = Map::LoadMap(ACTIVE_MAP);
    m_eventBus = &eventBus;
    SetupBulletSystem();
    SetupWallManager();
}

void GameSimulation::Reset() {
    m_gameTime = MATCH_TIME;
    m_players.fill(state::PlayerState{});
    m_bulletSystem.Reset();
    m_wallManager.Reset();
}

void GameSimulation::SetupBulletSystem() {
    m_bulletSystem.Initialize(*m_eventBus);
    m_bulletSystem.SetMap(m_map);

    m_bulletSystem.SetOnWallHit([this](Map::Vector2i gridPos, float damage, uint32_t shooterId) {
        m_wallManager.DamageWall(gridPos, damage, shooterId);
    });

    m_bulletSystem.SetOnPlayerHit([this](uint32_t playerId, float damage, uint32_t shooterId) {
        auto &player = m_players[playerId];

        if (player.invincibilityTimer > 0.0f)
            return;

        player.health -= damage;
        if (player.health <= 0.0f) {
            HandlePlayerDied(player, shooterId);
            return;
        }
        m_eventBus->publish(event::PlayerDamagedEvent{player.id, shooterId, player.health});
    });

    m_bulletSystem.SetOnBulletSpawn([this](uint32_t bulletId, uint32_t ownerId, Character::CharacterId characterId,
                                           Vector2 position, Vector2 velocity, uint32_t bulletPredSequence) {
        m_eventBus->publish(
            event::BulletSpawnEvent{bulletId, ownerId, characterId, bulletPredSequence, position, velocity});
    });
    m_bulletSystem.SetOnBulletDestroyed(
        [this](uint32_t bulletId, Vector2 position, Character::CharacterId characterId) {
            m_eventBus->publish(event::BulletDestroyedEvent{bulletId, position, characterId});
        });
}

void GameSimulation::HandlePlayerDied(state::PlayerState &player, uint32_t shooterId) {
    player.state = state::State::Dead;
    player.health = 0.0f;
    player.respawnTimer = RESPAWN_TIME;
    auto &killer = m_players[shooterId];
    player.score.deaths++;
    killer.score.kills++;
    m_eventBus->publish(event::PlayerDiedEvent{player, killer});
}

void GameSimulation::SetupWallManager() {
    m_wallManager.SetOnWallPlaced([this](Map::Vector2i gridPos, float health, const state::PlayerState &player) {
        m_eventBus->publish(event::PlaceWallEvent{gridPos, health, player});
    });

    m_wallManager.SetOnWallDamaged([this](Map::Vector2i gridPos, float currentHealth, uint32_t ownerId) {
        m_eventBus->publish(event::DamageWallEvent{gridPos, currentHealth, ownerId});
    });

    m_wallManager.SetOnWallDestroyed([this](Map::Vector2i gridPos, uint32_t ownerId) {
        state::PlayerState &player = m_players[ownerId];
        player.currentAvaliableWalls++;
        m_eventBus->publish(event::DestroyWallEvent{gridPos, player});
    });
    m_wallManager.SetOnWallPickedUp([this](Map::Vector2i gridPos, uint32_t ownerId) {
        state::PlayerState &player = m_players[ownerId];
        player.currentAvaliableWalls++;
        m_eventBus->publish(event::WallPickedUpEvent{gridPos, player});
    });
}

void GameSimulation::Update(float tickRate) {
    std::vector<Collision::AABB> dynamicColliders = m_wallManager.GetColliders();
    for (auto &player : m_players) {
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

        Character::SimulateMove(player, tickRate, m_map.walls, dynamicColliders);
    }

    m_bulletSystem.Update(tickRate, m_players, m_wallManager.GetAllWalls());
    m_gameTime -= tickRate;
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

    Vector2 moveInput = {player.currentInput.moveX, player.currentInput.moveY};
    bool isMoving = (moveInput.x * moveInput.x + moveInput.y * moveInput.y) > 0.0001f;

    if (isMoving) {
        player.state = state::State::Running;
    } else {
        player.state = state::State::Idle;
    }

    const Character::CharacterDef &charDef = Character::GetCharacterDef(characterId);
    bool shootNow = input.buttons & (1 << 0);
    bool shootPrev = player.lastButtons & (1 << 0);
    if (shootNow && !shootPrev && player.shootTimer <= 0.0f) {
        Vector2 aimDir = {input.aimX, input.aimY};
        m_bulletSystem.Spawn({player.id, player.position, aimDir, charDef, input.predBulletSequence});
        player.shootTimer = charDef.bullet.cooldown;
    }
    bool placeNow = input.buttons & (1 << 1);
    bool placePrev = player.lastButtons & (1 << 1);

    // Picking up and placing walls are on the same wall timer
    if (placeNow && !placePrev && player.wallTimer <= 0.0f) {
        HandleWallInput(player, input, charDef);
    }

    player.currentInput = input;
    player.lastButtons = input.buttons;
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
