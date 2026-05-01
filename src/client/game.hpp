#pragma once

#include "event_hub.hpp"
#include "scenes/scene_manager.hpp"
#include "session_manager.hpp"

class Game {
  public:
    void Run();

  private:
    void CreateWindow();
    void Shutdown();

  private:
    SceneManager m_sceneManager;
    SessionManager m_session{m_sceneManager};
    Client::EventHub m_events;
};
