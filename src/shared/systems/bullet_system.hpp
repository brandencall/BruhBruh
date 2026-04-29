#pragma once
#include "../../config.hpp"
#include "../../shared/characters/character_roster.hpp"
#include "../../shared/characters/character_types.hpp"
#include "../../shared/map/map_types.hpp"
#include "../components/collision.hpp"
#include "../map/dynamic_walls/dynamic_wall.hpp"
#include "../map/dynamic_walls/wall_manager.hpp"
#include "../state/bullet_state.hpp"
#include "../state/player_state.hpp"
#include "raylib.h"
#include <array>
#include <cstdint>
#include <unordered_map>

namespace System {

struct BulletSpawnDef {
    uint32_t ownerId;
    Vector2 position;
    Vector2 direction;
    const Character::CharacterDef &character;
};

template <typename TBulletState> class BulletSystem {
  public:
    BulletSystem() = default;

    virtual int Spawn(const BulletSpawnDef &bulletDef) {

        if (Vector2LengthSqr(bulletDef.direction) < 0.0001f)
            return -1;

        for (int i = 0; i < MAX_BULLETS; i++) {
            if (m_bullets[i].active)
                continue;

            uint16_t gen = ++m_generations[i];
            uint32_t id = MakeId(i, gen);
            Vector2 velocity = Vector2Scale(Vector2Normalize(bulletDef.direction), bulletDef.character.bullet.speed);
            InitBulletSlot(i, id, bulletDef.ownerId, bulletDef.position, velocity, bulletDef.character);

            OnBulletSpawn(m_bullets[i].id, bulletDef.ownerId, bulletDef.character.id, bulletDef.position,
                          m_bullets[i].velocity);
            return i;
        }
        return -1;
    }

    virtual int SpawnFromServerEvent(uint32_t serverId, uint32_t ownerId, Vector2 position, Vector2 velocity,
                                     const Character::CharacterDef &character) {
        int slot = GetSlot(serverId);
        if (slot < 0 || slot >= MAX_BULLETS)
            return -1;

        InitBulletSlot(slot, serverId, ownerId, position, velocity, character);

        return slot;
    }

    void InitBulletSlot(int slot, uint32_t id, uint32_t ownerId, Vector2 position, Vector2 velocity,
                        const Character::CharacterDef &character) {
        component::Hitbox hitbox = {
            .circle = {.center = {position.x, position.y}, .radius = character.bullet.radius},
            .damage = character.bullet.damage,
        };
        m_bullets[slot] = TBulletState{};
        m_bullets[slot].id = id;
        m_bullets[slot].ownerId = ownerId;
        m_bullets[slot].characterId = character.id;
        m_bullets[slot].velocity = velocity;
        m_bullets[slot].lifetime = character.bullet.lifetime;
        m_bullets[slot].rotation = atan2f(velocity.y, velocity.x) * RAD2DEG;
        m_bullets[slot].hitbox = hitbox;
        m_bullets[slot].active = true;
        OnSpawn(m_bullets[slot], position, character.id);
    }

    void Update(float dt, std::array<state::PlayerState, MAX_PLAYERS> &players,
                std::unordered_map<Map::Vector2i, Map::DynamicWall, Map::GridHash> &dynamicWalls) {
        for (auto &bullet : m_bullets) {
            if (!bullet.active)
                continue;

            UpdateBulletKinematics(bullet, dt);

            if (bullet.lifetime <= 0.0f) {
                Deactivate(bullet.id);
                continue;
            }

            HandleCollisions(bullet, players, dynamicWalls);
        }
    }

    void UpdateBulletKinematics(TBulletState &bullet, float dt) {
        const auto &def = Character::GetCharacterDef(bullet.characterId);

        bullet.hitbox.circle.center.x += bullet.velocity.x * dt;
        bullet.hitbox.circle.center.y += bullet.velocity.y * dt;

        float dir = (bullet.velocity.x >= 0.0f) ? 1.0f : -1.0f;
        bullet.rotation += dir * def.bullet.spinSpeed * dt;

        bullet.lifetime -= dt;
    }

    void HandleCollisions(TBulletState &bullet, const std::array<state::PlayerState, MAX_PLAYERS> &players,
                          const std::unordered_map<Map::Vector2i, Map::DynamicWall, Map::GridHash> &dynamicWalls) {
        if (m_map) {
            for (auto &wall : m_map->walls) {
                if (Collision::Overlap(bullet.hitbox.circle, wall)) {
                    Deactivate(bullet.id);
                    break;
                }
            }
        }
        for (auto &[_, wall] : dynamicWalls) {
            if (Collision::Overlap(bullet.hitbox.circle, wall.collider)) {
                Deactivate(bullet.id);
                OnWallHit(wall.gridPos, bullet.hitbox.damage, bullet.ownerId);
                break;
            }
        }

        for (auto &player : players) {
            if (player.respawnTimer <= 0.0f && bullet.ownerId != player.id && player.active &&
                Collision::Overlap(bullet.hitbox.circle, Collision::GetHurtBox(player))) {
                Deactivate(bullet.id);
                OnPlayerHit(player.id, bullet.hitbox.damage, bullet.ownerId);
                break;
            }
        }
    }

    void SetMap(const Map::MapData &map) { m_map = &map; }

    virtual void Deactivate(uint32_t id) {
        int slot = GetSlot(id);
        if (slot >= 0 && slot < MAX_BULLETS) {
            m_bullets[slot].active = false;
            OnBulletDestroyed(id);
        }
    }

    std::array<TBulletState, MAX_BULLETS> &GetBullets() { return m_bullets; }

    const state::BulletState *Get(int slot) const {
        if (slot < 0 || slot >= MAX_BULLETS || !m_bullets[slot].active)
            return nullptr;
        return &m_bullets[slot];
    }

    void Reset() {
        m_bullets.fill(TBulletState{});
        m_generations.fill(0);
    }

  protected:
    virtual void OnWallHit(Map::Vector2i gridPos, float damage, uint32_t shooterId) {}
    virtual void OnPlayerHit(uint32_t playerId, float damage, uint32_t shooterId) {}
    virtual void OnBulletSpawn(uint32_t bulletId, uint32_t ownerId, Character::CharacterId characterId,
                               Vector2 position, Vector2 velocity) {}
    virtual void OnBulletDestroyed(uint32_t bulletId) {}

  protected:
    virtual void OnSpawn(TBulletState &bullet, Vector2 spawnPos, Character::CharacterId characterId) {}

    static uint32_t MakeId(int slot, uint16_t generation) {
        return (static_cast<uint32_t>(generation) << 16) | static_cast<uint32_t>(slot);
    }

    static int GetSlot(uint32_t id) { return static_cast<int>(id & 0xFFFF); }

    std::array<TBulletState, MAX_BULLETS> m_bullets{};
    std::array<uint16_t, MAX_BULLETS> m_generations{}; // for stale ID detection

  private:
    const Map::MapData *m_map = nullptr;
};
} // namespace System
