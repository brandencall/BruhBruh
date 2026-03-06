#pragma once
#include "../../config.hpp"
#include "../components/shapes.hpp"
#include "../events.hpp"
#include "../state/bullet_state.hpp"
#include "../state/player_state.hpp"
#include "raymath.h"
#include <array>
#include <vector>

namespace System {

template <typename TBulletState> class BulletSystem {
  public:
    virtual int Spawn(uint32_t ownerId, Vector2 position, Vector2 direction, float speed = 400.0f,
                      float lifetime = 3.0f) {

        if (Vector2LengthSqr(direction) < 0.0001f)
            return -1;

        for (int i = 0; i < MAX_BULLETS; i++) {
            if (m_bullets[i].active)
                continue;

            uint16_t gen = ++m_generations[i];
            component::Hitbox hitbox = {
                .circle =
                    {
                        .x = position.x,
                        .y = position.y,
                        .radius = 4.0f,
                    },
                .damage = 10.0f,
            };

            m_bullets[i] = TBulletState{};
            m_bullets[i].id = MakeId(i, gen);
            m_bullets[i].ownerId = ownerId;
            m_bullets[i].velocity = Vector2Scale(Vector2Normalize(direction), speed);
            m_bullets[i].lifetime = lifetime;
            m_bullets[i].hitbox = hitbox;
            m_bullets[i].active = true;
            m_spawnEvents.emplace_back(m_bullets[i].id, ownerId, position, m_bullets[i].velocity, lifetime);
            OnSpawn(m_bullets[i], position);

            return i;
        }
        return -1;
    }

    virtual int SpawnWithId(uint32_t serverId, uint32_t ownerId, Vector2 position, Vector2 velocity,
                            float lifetime = 3.0f) {
        int slot = GetSlot(serverId);
        if (slot < 0 || slot >= MAX_BULLETS)
            return -1;

        component::Hitbox hitbox = {
            .circle = {.x = position.x, .y = position.y, .radius = 4.0f},
            .damage = 10.0f,
        };

        m_bullets[slot] = TBulletState{};
        m_bullets[slot].id = serverId; // use server's ID directly
        m_bullets[slot].ownerId = ownerId;
        m_bullets[slot].velocity = velocity;
        m_bullets[slot].lifetime = lifetime;
        m_bullets[slot].hitbox = hitbox;
        m_bullets[slot].active = true;
        OnSpawn(m_bullets[slot], position);

        return slot;
    }

    virtual void Update(float dt, std::array<state::PlayerState, MAX_PLAYERS> &players) {
        for (auto &bullet : m_bullets) {
            if (!bullet.active)
                continue;

            bullet.hitbox.circle.x += bullet.velocity.x * dt;
            bullet.hitbox.circle.y += bullet.velocity.y * dt;
            bullet.lifetime -= dt;

            if (bullet.lifetime <= 0.0f) {
                Deactivate(bullet.id);
                continue;
            }
            for (auto &player : players) {
                if (bullet.ownerId != player.id && Shapes::Overlap(bullet.hitbox.circle, Shapes::GetHurtBox(player))) {
                    player.health -= bullet.hitbox.damage;
                    bullet.active = false;
                    m_hitEvents.emplace_back(bullet.id, player.id,
                                             Vector2{bullet.hitbox.circle.x, bullet.hitbox.circle.y});
                    continue;
                }
            }
        }
    }

    virtual void Deactivate(uint32_t id) {
        int slot = GetSlot(id);
        if (slot >= 0 && slot < MAX_BULLETS) {
            m_bullets[slot].active = false;
            m_expireEvents.emplace_back(id);
        }
    }

    std::array<TBulletState, MAX_BULLETS> &GetBullets() { return m_bullets; }

    const state::BulletState *Get(int slot) const {
        if (slot < 0 || slot >= MAX_BULLETS || !m_bullets[slot].active)
            return nullptr;
        return &m_bullets[slot];
    }

    void clearEvents() {
        m_spawnEvents.clear();
        m_hitEvents.clear();
        m_expireEvents.clear();
    }

  public:
    // TODO: Add a despawn event
    std::vector<event::BulletSpawnEvent> m_spawnEvents;
    std::vector<event::BulletHitEvent> m_hitEvents;
    std::vector<event::BulletExpireEvent> m_expireEvents;

  protected:
    virtual void OnSpawn(TBulletState &bullet, Vector2 spawnPos) {}

    static uint32_t MakeId(int slot, uint16_t generation) {
        return (static_cast<uint32_t>(generation) << 16) | static_cast<uint32_t>(slot);
    }

    static int GetSlot(uint32_t id) { return static_cast<int>(id & 0xFFFF); }

    std::array<TBulletState, MAX_BULLETS> m_bullets{};
    std::array<uint16_t, MAX_BULLETS> m_generations{}; // for stale ID detection
};
} // namespace System
