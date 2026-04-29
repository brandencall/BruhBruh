#pragma once
#include "../event_hub.hpp"
#include "../session_manager.hpp"
#include "scene.hpp"
#include "scene_manager.hpp"

class JoinScene : public Scene {
  public:
    JoinScene(Client::EventHub &events, SessionManager &sessionManager, SceneManager &sceneManager);

    void OnEnter() override;
    void OnExit() override;
    void Update(float dt) override;
    void Render() override;

  private:
    Client::EventHub &m_events;
    SessionManager &m_sessionManager;
    SceneManager &sceneManager;
};
