#include "bullet_system.hpp"
#include "player.hpp"
#include "raylib.h"
#include <algorithm>

BulletSystem::BulletSystem() {}

void BulletSystem::Update(float dt, const Player &player, const Camera2D &camera) {
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), camera);
        Vector2 playerPos = player.GetPosition();

        Vector2 dir = Vector2Subtract(mouseWorldPos, playerPos);

        m_bullets.emplace_back(playerPos, dir);
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
