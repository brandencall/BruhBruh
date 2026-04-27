#pragma once
#include "client/game_client.hpp"
#include "server/game_server.hpp"
#include "shared/network/steam_lobby_manager.hpp"
#include "shared/network/steam_transport.hpp"
#include <thread>

// Owns both server and client for the hosting player.
// Clients (non-host) only use GameClient directly.
class GameHost {
  public:
    GameHost() : m_lobbyManager(m_transport) {}

    // Called when this player is creating the lobby
    void StartHost() {
        // Server runs on background thread
        m_serverThread = std::thread([this]() {
            m_server.StartInProcess(m_transport);
            m_server.RunServer();
        });
        m_serverThread.detach();

        // Client connects to local server via same SteamTransport
        m_client.Initialize();
        m_client.StartInProcess(m_transport, m_lobbyManager);
    }

    SteamLobbyManager &GetLobbyManager() { return m_lobbyManager; }

  private:
    network::SteamTransport m_transport;
    SteamLobbyManager m_lobbyManager;
    GameServer m_server;
    GameClient m_client;
    std::thread m_serverThread;
};
