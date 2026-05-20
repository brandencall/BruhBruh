#pragma once
#include "../server/game_server.hpp"
#include "../shared/network/steam_lobby_manager.hpp"
#include "../shared/state/lobby_slot_state.hpp"
#include "event_hub.hpp"
#include "game_client.hpp"
#include "network/network_message_handler.hpp"
#include "scenes/scene_manager.hpp"
#include "steam/steamclientpublic.h"
#include <functional>
#include <memory>
#include <thread>

class SessionManager {
  public:
    using LobbyFactory = std::function<void()>;
    using GameSceneFactory = std::function<void(const state::LobbySlotState &currentPlayerState)>;

    SessionManager(SceneManager &sceneManager);

    void SetCreateLobbyFactory(LobbyFactory f);
    void SetReplaceLobbyFactory(LobbyFactory f);
    void CreateLobby();
    void ReplaceSceneWithLobby();
    void SetGameSceneFactory(GameSceneFactory f);
    void CreateGame(const state::LobbySlotState &currentPlayerState);

    void Initialize();
    void InitializeClientTransport(network::ITransport &transport, NetworkMessageHandler &handler);
    void Shutdown();

    network::ITransport *GetTransport();
    NetworkMessageHandler *GetHandler();
    void TickClient();
    void HostGame();
    void JoinLobby(CSteamID lobbyId);
    void ReturnToMainMenu();

    void HostGame(std::function<void()> onSuccess, std::function<void(const char *)> onError);
    // Joins an existing lobby by ID, calls onSuccess/onError when done
    void JoinLobby(CSteamID lobbyId, std::function<void()> onSuccess, std::function<void(const char *)> onError);

    // Remove below methods
    SteamLobbyManager *GetLobby();

  private:
    void StartServerThread();
    void StartGameClient();
    void ShutdownServer();

  private:
    // Remove the below members
    Client::EventHub m_events;
    bool m_shouldStartHost = false;
    bool m_returningToStart = false;

    LobbyFactory m_createLobby;
    LobbyFactory m_replaceSceneWithLobby;
    GameSceneFactory m_createGameScene;

    SceneManager &m_sceneManager;
    NetworkMessageHandler *m_handler = nullptr;
    std::unique_ptr<SteamLobbyManager> m_lobbyManager;
    network::ITransport *m_transport = nullptr;
    std::unique_ptr<GameClient> m_client;
    std::unique_ptr<GameServer> m_server;
    std::thread m_serverThread;
};
