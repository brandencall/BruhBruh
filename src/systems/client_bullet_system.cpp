#include "client_bullet_system.hpp"

namespace System {
std::array<state::ClientBulletState, MAX_BULLETS> &ClientBulletSystem::GetBullets() { return m_bullets; }

void ClientBulletSystem::Update(float dt) {

    constexpr float BULLET_INTERP_SPEED = 20.0f;
    for (auto &bullet : m_bullets) {
        if (!bullet.active)
            continue;
        bullet.position.x += bullet.velocity.x * dt;
        bullet.position.y += bullet.velocity.y * dt;
        bullet.lifetime -= dt;
        if (bullet.lifetime <= 0.0f) {
            bullet.active = false;
            continue;
        }
        bullet.position = Vector2Lerp(bullet.position, bullet.serverPosition, BULLET_INTERP_SPEED * dt);
    }
}

int ClientBulletSystem::Spawn(uint32_t ownerId, Vector2 position, Vector2 direction, float speed, float lifetime) {
    if (Vector2LengthSqr(direction) < 0.0001f)
        return -1;

    for (int i = 0; i < MAX_BULLETS; i++) {
        if (m_bullets[i].active)
            continue;

        uint16_t gen = ++m_generations[i];

        m_bullets[i] = state::ClientBulletState{};
        m_bullets[i].id = MakeId(i, gen);
        m_bullets[i].ownerId = ownerId;
        m_bullets[i].position = position;
        m_bullets[i].velocity = Vector2Scale(Vector2Normalize(direction), speed);
        m_bullets[i].lifetime = lifetime;
        m_bullets[i].active = true;
        m_bullets[i].serverPosition = position; // initialize to spawn position

        return i;
    }
    return -1;
}

void ClientBulletSystem::Deactivate(int slot) {
    if (slot >= 0 && slot < MAX_BULLETS)
        m_bullets[slot].active = false;
}

} // namespace System
