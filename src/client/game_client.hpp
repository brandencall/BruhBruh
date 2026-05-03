#pragma once
#include "../client_transport.hpp"
#include "../shared/network/ITransport.hpp"
#include "../shared/network/steam_lobby_manager.hpp"
#include "client_transport.hpp"
#include "network/network_message_handler.hpp"
#include "scenes/scene_manager.hpp"

class GameClient {
  public:
    GameClient(network::ITransport &transport, SteamLobbyManager &lobbyManager, NetworkMessageHandler &handler);
    ~GameClient();
    void Start(const char *ip, int port);
    void StartInProcess();
    void Update();
    void Disconnect();

  private:
    network::ClientTransport m_ownedTransport;
    network::ITransport *m_transport = nullptr;
    SteamLobbyManager &m_lobbyManager;
    NetworkMessageHandler &m_handler;
    SceneManager m_sceneManager;

    bool m_running = false;
};
