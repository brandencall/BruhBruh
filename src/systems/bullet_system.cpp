#include "bullet_system.hpp"
#include "bullet_entity.hpp"
#include "damage_system.hpp"
#include "raylib.h"

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

} // namespace System
