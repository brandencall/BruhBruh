#pragma once
#include "../network/client.hpp"
#include "../network/server.hpp"
#include "game_simulation.hpp"
#include <array>

class GameServer {
  public:
    GameServer();
    void Start(int port);
    bool IsRunning();
    void Receive(GameSimulation &simulation);
    // Temp method
    void Receive();
    void HandlePacket(char *buffer, size_t bytes, sockaddr_in &clientAddr);
    void HandleJoin(const sockaddr_in &addr);
    void BroadcastState(GameSimulation &simulation);
    network::ClientConnection *FindClient(const sockaddr_in &addr);

  private:
    static constexpr int MAX_PLAYERS = 4;
    int m_nextPlayerId = 1;
    bool m_running = false;
    network::Server m_server;

    std::array<network::ClientConnection, MAX_PLAYERS> m_clients;
};
