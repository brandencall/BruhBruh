#pragma once
#include "bullet_system.hpp"
#include "player.hpp"
#include <raylib.h>

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
    Player m_player;
    Camera2D m_camera;
    BulletSystem m_bulletSystem;
};
