#pragma once
#include "../server/game_server.hpp"
#include "../shared/network/steam_lobby_manager.hpp"
#include "../shared/state/lobby_slot_state.hpp"
#include "event_hub.hpp"
#include "game_client.hpp"
#include "network/network_message_handler.hpp"
#include "scene_manager.hpp"
#include "steam/steamclientpublic.h"
#include <memory>
#include <thread>

class SessionManager {
  public:
    SessionManager(SceneManager &sceneManager);

    void Initialize();
    void Shutdown();

    network::ITransport &GetTransport();
    NetworkMessageHandler &GetHandler();
    void TickClient();
    void HostGame();
    void JoinLobby(CSteamID lobbyId);
    void CreateGame(const state::LobbySlotState &currentPlayerState);
    void ReturnToStart();

    void HostGame(std::function<void()> onSuccess, std::function<void(const char *)> onError);
    // Joins an existing lobby by ID, calls onSuccess/onError when done
    void JoinLobby(CSteamID lobbyId, std::function<void()> onSuccess, std::function<void(const char *)> onError);

    // Remove below methods
    SteamLobbyManager *GetLobby();
    void CreateLobby();

  private:
    void StartServerThread();
    void StartGameClient();
    void ShutdownServer();

  private:
    // Remove the below members
    Client::EventHub m_events;
    bool m_shouldStartHost = false;
    bool m_returningToStart = false;

    SceneManager &m_sceneManager;
    NetworkMessageHandler m_handler;
    std::unique_ptr<SteamLobbyManager> m_lobbyManager;
    std::unique_ptr<network::SteamTransport> m_transport;
    std::unique_ptr<GameClient> m_client;
    std::unique_ptr<GameServer> m_server;
    std::thread m_serverThread;
};
