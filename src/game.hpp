#pragma once
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
    bool m_debugMode = true;
    Player m_player;
    Camera2D m_camera;
};
