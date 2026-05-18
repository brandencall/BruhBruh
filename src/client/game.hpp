#pragma once

#include "game_client.hpp"
#include "network/network_message_handler.hpp"
#include "scenes/scene_manager.hpp"
#include "session_manager.hpp"
#include "systems/audio_system.hpp"

class Game {
  public:
    void Run();
    void RequestQuit();
    void RunLocal(GameClient &client);

    SessionManager *GetSessionManager();
    SceneManager *GetSceneManager();
    System::AudioSystem *GetAudioSystem();
    NetworkMessageHandler *GetNetworkMessageHandler();
    network::ITransport *GetTransport();

  private:
    void CreateWindow();
    void Shutdown();

  private:
    SceneManager m_sceneManager;
    System::AudioSystem m_audioSystem;
    SessionManager m_session{m_sceneManager};
    bool m_shouldQuit = false;
};
