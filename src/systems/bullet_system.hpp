#pragma once
#include "bullet_entity.hpp"
#include "player.hpp"
#include <raylib.h>
#include <vector>

class BulletSystem {
  public:
    BulletSystem();
    void Update(float dt, const Player &player, const Camera2D &camera);
    void Draw();

  private:
    std::vector<BulletEntity> m_bullets;
};
