#pragma once
#include "bullet_entity.hpp"
#include "entity.hpp"
#include <memory>
#include <raylib.h>
#include <vector>

namespace System {
class DamageSystem;

class BulletSystem {
  public:
    std::unique_ptr<BulletEntity> CreateBullet(Entity &entity, const Camera2D &camera);
    void Update(float dt, std::vector<std::unique_ptr<Entity>> &entities);
    void Draw(std::vector<std::unique_ptr<Entity>> &entities);
};
} // namespace System
