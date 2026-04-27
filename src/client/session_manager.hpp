#pragma once
#include "../server/game_server.hpp"
#include "../shared/network/steam_lobby_manager.hpp"
#include "../shared/state/lobby_slot_state.hpp"
#include "event_hub.hpp"
#include "game_client.hpp"
#include "network/network_message_handler.hpp"
#include "scene_manager.hpp"
#include <memory>
#include <thread>

class SessionManager {
  public:
    SessionManager() = default;

    void Initialize();
    void Shutdown();

    void StartHost();
    void StartClient();
    void Run();
    SteamLobbyManager &GetLobby();
    void CreateLobby();
    void CreateGame(const state::LobbySlotState &currentPlayerState);

  private:
    void StartGamerServerThread();
    void StartGameClient();

  private:
    Client::EventHub m_events;
    NetworkMessageHandler m_handler;
    SceneManager m_sceneManager;

    std::unique_ptr<SteamLobbyManager> m_lobbyManager;
    std::unique_ptr<network::SteamTransport> m_transport;
    std::unique_ptr<GameClient> m_client;
    std::unique_ptr<GameServer> m_server;
    std::thread m_serverThread;
    bool m_shouldStartHost = false;
};
