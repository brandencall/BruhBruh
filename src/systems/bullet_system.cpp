#include "bullet_system.hpp"
#include "raylib.h"
#include <algorithm>

BulletSystem::BulletSystem() {}

void BulletSystem::Update(float dt, const Entity &entity, const Camera2D &camera) {
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), camera);
        Vector2 entityPos = entity.GetPosition();

        Vector2 dir = Vector2Subtract(mouseWorldPos, entityPos);

        m_bullets.emplace_back(entityPos, dir);
    }

    for (auto &bullet : m_bullets) {
        bullet.Update(dt);
    }
    m_bullets.erase(
        std::remove_if(m_bullets.begin(), m_bullets.end(), [](const BulletEntity &b) { return b.IsDead(); }),
        m_bullets.end());
}

void BulletSystem::Draw() {
    for (auto &bullet : m_bullets) {
        bullet.Draw();
    }
}
