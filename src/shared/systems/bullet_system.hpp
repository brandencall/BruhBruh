#pragma once
#include "../../config.hpp"
#include "../components/shapes.hpp"
#include "../state/bullet_state.hpp"
#include "../state/player_state.hpp"
#include "raymath.h"
#include <array>
#include <iostream>

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
            // m_bullets[i].position = position;
            m_bullets[i].velocity = Vector2Scale(Vector2Normalize(direction), speed);
            m_bullets[i].lifetime = lifetime;
            m_bullets[i].hitbox = hitbox;
            m_bullets[i].active = true;
            OnSpawn(m_bullets[i], position);

            return i;
        }
        return -1;
    }
    virtual void Update(float dt, std::array<state::PlayerState, MAX_PLAYERS> &players) {
        for (auto &bullet : m_bullets) {
            if (!bullet.active)
                continue;

            bullet.hitbox.circle.x += bullet.velocity.x * dt;
            bullet.hitbox.circle.y += bullet.velocity.y * dt;
            bullet.lifetime -= dt;

            if (bullet.lifetime <= 0.0f) {
                bullet.active = false;
                continue;
            }
            for (auto &player : players) {
                if (bullet.ownerId != player.id && Shapes::Overlap(bullet.hitbox.circle, Shapes::GetHurtBox(player))) {
                    std::cout << "Bullet hit player!" << std::endl;
                    player.health -= bullet.hitbox.damage;
                    bullet.active = false;
                    std::cout << "Player " << player.id << " health: " << player.health << std::endl;
                    continue;
                }
            }

            OnUpdate(bullet, dt);
        }
    }

    virtual void Deactivate(int slot) {
        if (slot >= 0 && slot < MAX_BULLETS)
            m_bullets[slot].active = false;
    }

    std::array<TBulletState, MAX_BULLETS> &GetBullets() { return m_bullets; }
    const state::BulletState *Get(int slot) const {
        if (slot < 0 || slot >= MAX_BULLETS || !m_bullets[slot].active)
            return nullptr;
        return &m_bullets[slot];
    }

  protected:
    virtual void OnSpawn(TBulletState &bullet, Vector2 spawnPos) {}
    virtual void OnUpdate(TBulletState &bullet, float dt) {}

    static uint32_t MakeId(int slot, uint16_t generation) {
        return (static_cast<uint32_t>(generation) << 16) | static_cast<uint32_t>(slot);
    }

    std::array<TBulletState, MAX_BULLETS> m_bullets{};
    std::array<uint16_t, MAX_BULLETS> m_generations{}; // for stale ID detection
};
} // namespace System
