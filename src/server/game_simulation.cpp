#include "game_simulation.hpp"
#include "../config.hpp"
#include "../shared/map/grid.hpp"
#include "../shared/map/map_loader.hpp"
#include "characters/character_roster.hpp"
#include "characters/character_types.hpp"
#include "events.hpp"
#include "raylib.h"
#include "state/player_state.hpp"
#include <cstdint>
#include <sys/types.h>

void GameSimulation::Initialize(EventBus &eventBus) {
    m_players = {};
    m_map = Map::LoadMap(ACTIVE_MAP);
    m_eventBus = &eventBus;
    SetupBulletSystem();
    SetupWallManager();
}

void GameSimulation::SetupBulletSystem() {
    m_bulletSystem.Initialize(*m_eventBus);
    m_bulletSystem.SetMap(m_map);

    m_bulletSystem.SetOnWallHit([this](Map::Vector2i gridPos, float damage, uint32_t shooterId) {
        m_wallManager.DamageWall(gridPos, damage, shooterId);
    });

    m_bulletSystem.SetOnPlayerHit([this](uint32_t playerId, float damage, uint32_t shooterId) {
        auto &player = m_players[playerId];
        player.health -= damage;
        if (player.health <= 0.0f) {
            HandlePlayerDied(player, shooterId);
            return;
        }
        m_eventBus->publish(event::PlayerDamagedEvent{player.id, player.health});
    });

    m_bulletSystem.SetOnBulletSpawn([this](uint32_t bulletId, uint32_t ownerId, Character::CharacterId characterId,
                                           Vector2 position, Vector2 velocity) {
        m_eventBus->publish(event::BulletSpawnEvent{bulletId, ownerId, characterId, position, velocity});
    });
    m_bulletSystem.SetOnBulletDestroyed(
        [this](uint32_t bulletId) { m_eventBus->publish(event::BulletDestroyedEvent{bulletId}); });
}

void GameSimulation::HandlePlayerDied(state::PlayerState &player, uint32_t shooterId) {
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

        Vector2 dir = Vector2Normalize({player.currentInput.moveX, player.currentInput.moveY});
        player.velocity = Vector2Scale(dir, player.speed);
        player.position = Vector2Add(player.position, Vector2Scale(player.velocity, tickRate));
        Collision::Circle circle = {player.position, player.hurtbox.radius};
        player.position = Collision::resolveCircleAABBList(circle, m_map.walls, dynamicColliders);
    }

    m_bulletSystem.Update(tickRate, m_players, m_wallManager.GetAllWalls());
}

void GameSimulation::RespawnPlayer(state::PlayerState &player) {
    const auto &def = GetCharacterDef(player.characterId);
    player.health = def.maxHealth;
    // TODO: Create a better respawn position based on other players positions and map bounds
    player.position = m_map.spawnPoints[player.id];
    player.velocity = {0, 0};
    player.respawnTimer = 0.0f;
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

    const Character::CharacterDef &charDef = Character::GetCharacterDef(characterId);
    bool shootNow = input.buttons & (1 << 0);
    bool shootPrev = player.lastButtons & (1 << 0);
    if (shootNow && !shootPrev) {
        Vector2 aimDir = {input.aimX, input.aimY};
        m_bulletSystem.Spawn(player.id, player.position, aimDir, charDef);
    }
    bool placeNow = input.buttons & (1 << 1);
    bool placePrev = player.lastButtons & (1 << 1);
    if (placeNow && !placePrev) {
        HandleWallInput(player, input);
    }

    player.currentInput = input;
    player.lastButtons = input.buttons;
}

void GameSimulation::HandleWallInput(state::PlayerState &player, const state::PlayerInput &input) {
    Vector2 worldPos = {input.aimX, input.aimY};
    Map::Vector2i gridPos = Map::WorldToGrid(worldPos);
    if (!TryPlaceWall(player, gridPos)) {
        int wallOwnerId = m_wallManager.GetOwnerId(gridPos);
        if (wallOwnerId == player.id) {
            m_wallManager.RemoveWall(gridPos, player.id);
        }
    }
}

bool GameSimulation::TryPlaceWall(state::PlayerState &player, Map::Vector2i gridPos) {
    if (!m_wallManager.CanPlaceWall(gridPos, m_map.walls, player, m_players))
        return false;

    player.currentAvaliableWalls--;
    m_wallManager.PlaceWall(gridPos, 20, player);
    return true;
}

const std::array<state::PlayerState, MAX_PLAYERS> &GameSimulation::GetPlayers() const { return m_players; }

void GameSimulation::CreatePlayer(uint32_t playerId, Character::CharacterId characterId) {
    if (playerId > MAX_PLAYERS)
        return;
    const Character::CharacterDef &charDef = GetCharacterDef(characterId);
    Vector2 spawn = m_map.spawnPoints[playerId];
    const auto &def = GetCharacterDef(characterId);
    state::PlayerState player = {.id = playerId,
                                 .characterId = characterId,
                                 .position = spawn,
                                 .speed = def.moveSpeed,
                                 .health = def.maxHealth,
                                 .hurtbox = {.radius = charDef.hurtboxRadius},
                                 .lastButtons = 0,
                                 .respawnTimer = 0.0f,
                                 .currentAvaliableWalls = 5,
                                 .active = true};
    // TODO: Take in the NAME on creation
    snprintf(player.name, sizeof(player.name), "player[%u]", playerId);
    m_players[playerId] = player;
}

void GameSimulation::RemovePlayer(uint32_t playerId) {
    if (playerId > MAX_PLAYERS)
        return;
    state::PlayerState &player = m_players[playerId];
    player.id = UINT32_MAX;
    player.active = false;
}
