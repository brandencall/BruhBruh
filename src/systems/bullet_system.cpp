#include "bullet_system.hpp"
#include "bullet_entity.hpp"
#include "damage_system.hpp"
#include "raylib.h"
#include <iostream>

namespace System {

std::unique_ptr<BulletEntity> BulletSystem::CreateBullet(Entity &entity, const Camera2D &camera) {
    Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), camera);
    Vector2 entityPos = entity.GetPosition();
    Vector2 dir = Vector2Subtract(mouseWorldPos, entityPos);

    return std::make_unique<BulletEntity>(entityPos, dir, &entity);
}

void BulletSystem::Update(float dt, std::vector<std::unique_ptr<Entity>> &entities) {
    for (auto &e : entities) {
        if (e->GetType() != BulletEntity::EntityType::Bullet)
            continue;

        e->Update(dt);
    }
}

void BulletSystem::Draw(std::vector<std::unique_ptr<Entity>> &entities) {
    for (auto &e : entities) {
        if (e->GetType() != BulletEntity::EntityType::Bullet)
            continue;

        e->Draw();
    }
}

int BulletSystem::Spawn(uint32_t ownerId, Vector2 position, Vector2 direction, float speed, float lifetime) {
    if (Vector2LengthSqr(direction) < 0.0001f)
        return -1;

    for (int i = 0; i < MAX_BULLETS; i++) {
        if (m_bullets[i].active)
            continue;

        uint16_t gen = ++m_generations[i];

        m_bullets[i] = state::BulletState{
            .id = MakeId(i, gen),
            .ownerId = ownerId,
            .position = position,
            .velocity = Vector2Scale(Vector2Normalize(direction), speed),
            .lifetime = lifetime,
            .active = true,
        };

        return i;
    }
    return -1;
}

void BulletSystem::Update(float dt) {
    for (auto &bullet : m_bullets) {
        if (!bullet.active)
            continue;

        bullet.position.x += bullet.velocity.x * dt;
        bullet.position.y += bullet.velocity.y * dt;
        bullet.lifetime -= dt;

        if (bullet.lifetime <= 0.0f)
            bullet.active = false;
    }
}

void BulletSystem::Deactivate(int slot) {
    if (slot >= 0 && slot < MAX_BULLETS)
        m_bullets[slot].active = false;
}

const state::BulletState *BulletSystem::Get(int slot) const {
    if (slot < 0 || slot >= MAX_BULLETS || !m_bullets[slot].active)
        return nullptr;
    return &m_bullets[slot];
}

} // namespace System
