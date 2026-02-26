#pragma once
#include "bullet_entity.hpp"
#include "entity.hpp"
#include <raylib.h>
#include <vector>

class BulletSystem {
  public:
    BulletSystem();
    void Update(float dt, const Entity &entity, const Camera2D &camera);
    void Draw();

  private:
    std::vector<BulletEntity> m_bullets;
};
