#pragma once
#include "bullet_system.hpp"
#include "collision_system.hpp"
#include "damage_system.hpp"
#include "entity.hpp"
#include "player.hpp"
#include "raylib.h"
#include "temp_entity.hpp"
#include "temp_wall.hpp"
#include <memory>
#include <raylib.h>
#include <vector>

class Game {
  public:
    Game();
    ~Game();

    bool GameRunning();
    void Update();

  private:
    void Draw();
    void DrawDebugGrid();
    void SetGameRunning(bool runningState);

  private:
    bool m_running = true;
    Camera2D m_camera;
    System::DamageSystem m_damageSystem;
    System::BulletSystem m_bulletSystem;
    System::CollisionSystem m_collisionSystem;
    std::vector<std::unique_ptr<Entity>> m_entities;
    Player *m_player = nullptr;
    // TESTING
    Wall *m_wall = nullptr;
    TestEntity *m_testEntity = nullptr;
    // -----------------
};
