#include "game_simulation.hpp"
#include "../config.hpp"
#include "../shared/map/grid.hpp"
#include "../shared/map/map_loader.hpp"
#include "characters/character_roster.hpp"
#include "characters/character_types.hpp"
#include "events.hpp"
#include "raylib.h"
#include <cstdint>
#include <iostream>
#include <sys/types.h>

void GameSimulation::Initialize(EventBus &eventBus) {
    m_players = {};
    m_map = Map::LoadMap(MAP_PATH);
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
            player.health = 0.0f;
            player.respawnTimer = RESPAWN_TIME;
            m_eventBus->publish(event::PlayerDiedEvent{player.id, player.characterId, player.respawnTimer});
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

void GameSimulation::SetupWallManager() {
    m_wallManager.SetOnWallPlaced([this](Map::Vector2i gridPos, float health, uint32_t ownerId) {
        m_eventBus->publish(event::PlaceWallEvent{gridPos, health, ownerId});
    });

    m_wallManager.SetOnWallDamaged([this](Map::Vector2i gridPos, float currentHealth, uint32_t ownerId) {
        m_eventBus->publish(event::DamageWallEvent{gridPos, currentHealth, ownerId});
    });

    m_wallManager.SetOnWallDestroyed([this](Map::Vector2i gridPos, uint32_t ownerId) {
        m_eventBus->publish(event::DestroyWallEvent{gridPos, ownerId});
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
}

const std::array<state::BulletState, MAX_BULLETS> &GameSimulation::GetBullets() { return m_bulletSystem.GetBullets(); }

System::BulletSystem<state::BulletState> &GameSimulation::GetBulletSystem() { return m_bulletSystem; }

Map::WallManager &GameSimulation::GetWallManager() { return m_wallManager; }

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
        Vector2 worldPos = {input.aimX, input.aimY};
        Map::Vector2i gridPos = Map::WorldToGrid(worldPos);
        if (m_wallManager.CanPlaceWall(gridPos, m_map.walls, m_players)) {
            m_wallManager.PlaceWall(gridPos, 20, playerId);
            std::cout << "Player " << playerId << " placed a wall at position (" << worldPos.x << ", " << worldPos.y
                      << ")." << std::endl;
        } else {
            std::cout << "Player " << playerId << " failed to place a wall at position (" << worldPos.x << ", "
                      << worldPos.y << ")." << std::endl;
        }
    }

    player.currentInput = input;
    player.lastButtons = input.buttons;
}

const std::array<state::PlayerState, MAX_PLAYERS> &GameSimulation::GetPlayers() { return m_players; }

void GameSimulation::CreatePlayer(uint32_t playerId, Character::CharacterId characterId) {
    if (playerId > MAX_PLAYERS)
        return;
    const Character::CharacterDef &charDef = GetCharacterDef(characterId);
    Vector2 spawn = m_map.spawnPoints[playerId];
    state::PlayerState player = {.id = playerId,
                                 .characterId = characterId,
                                 .position = spawn,
                                 .hurtbox = {.radius = charDef.hurtboxRadius},
                                 .lastButtons = 0,
                                 .respawnTimer = 0.0f,
                                 .active = true};
    m_players[playerId] = player;
}

void GameSimulation::RemovePlayer(uint32_t playerId) {
    if (playerId > MAX_PLAYERS)
        return;
    state::PlayerState &player = m_players[playerId];
    player.id = UINT32_MAX;
    player.active = false;
}
