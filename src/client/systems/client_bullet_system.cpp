#include "client_bullet_system.hpp"
#include "raylib.h"

namespace System {

void ClientBulletSystem::Update(float dt, std::array<state::PlayerState, MAX_PLAYERS> &players) {
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
    }
}

void ClientBulletSystem::OnSpawn(state::ClientBulletState &bullet, Vector2 spawnPos) {
    bullet.serverPosition = spawnPos;
}

void ClientBulletSystem::AssignId(int slot, uint32_t id) {
    if (slot < 0 || slot >= MAX_BULLETS)
        return;
    if (!m_bullets[slot].active)
        return;
    m_bullets[slot].id = id;
}

} // namespace System
