#include "bullet_system.hpp"
#include "damage_system.hpp"
#include "raylib.h"
#include <algorithm>

namespace System {
BulletSystem::BulletSystem(DamageSystem &damageSystem) : m_damageSystem(damageSystem) {}

void BulletSystem::SpawnBullet(Entity &entity, const Camera2D &camera) {
    Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), camera);
    Vector2 entityPos = entity.GetPosition();

    Vector2 dir = Vector2Subtract(mouseWorldPos, entityPos);

    m_bullets.emplace_back(entityPos, dir, &entity);
    m_damageSystem.AddHitbox(m_bullets.back().getHitbox());
}

void BulletSystem::Update(float dt) {
    for (auto &bullet : m_bullets) {
        bullet.Update(dt);
    }

    for (auto &bullet : m_bullets)
        if (bullet.IsDead())
            m_damageSystem.RemoveHitbox(bullet.getHitbox());

    m_bullets.erase(
        std::remove_if(m_bullets.begin(), m_bullets.end(), [](const BulletEntity &b) { return b.IsDead(); }),
        m_bullets.end());
}

void BulletSystem::Draw() {
    for (auto &bullet : m_bullets) {
        bullet.Draw();
    }
}

} // namespace System
