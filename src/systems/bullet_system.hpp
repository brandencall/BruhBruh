#pragma once
#include "bullet_entity.hpp"
#include "entity.hpp"
#include <raylib.h>
#include <vector>

namespace System {
class DamageSystem;

class BulletSystem {
  public:
    BulletSystem(DamageSystem &damageSystem);
    void SpawnBullet(Entity &entity, const Camera2D &camera);
    void Update(float dt);
    void Draw();

  private:
    std::vector<BulletEntity> m_bullets;
    DamageSystem &m_damageSystem;
};
} // namespace System
