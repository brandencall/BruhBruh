#pragma once
#include "bullet_system.hpp"
#include "collision_system.hpp"
#include "entity.hpp"
#include "player.hpp"
#include "raylib.h"
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
    BulletSystem m_bulletSystem;
    CollisionSystem m_collisionSystem;
    std::vector<std::unique_ptr<Entity>> m_entities;
    Player *m_player = nullptr;
    // TESTING COLLISION
    Wall *m_wall = nullptr;
    // -----------------
};
