#pragma once

#include "game_client.hpp"
#include "network/network_message_handler.hpp"
#include "scenes/scene_manager.hpp"
#include "session_manager.hpp"

class Game {
  public:
    void Run();
    void RequestQuit();
    void RunLocal(GameClient &client, NetworkMessageHandler &handler);

  private:
    void CreateWindow();
    void Shutdown();

  private:
    SceneManager m_sceneManager;
    SessionManager m_session{m_sceneManager};
    bool m_shouldQuit = false;
};
