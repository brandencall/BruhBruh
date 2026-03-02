#pragma once
#include "../network/client.hpp"
#include "../network/server.hpp"
#include "game_simulation.hpp"
#include <array>

class GameServer {
  public:
    GameServer() = default;
    void Start(int port);
    void RunServer();
    bool IsRunning();

  private:
    void UpdateSimulation(float tickRate);
    void Receive();
    void BroadcastState();
    void HandlePacket(char *buffer, size_t bytes, sockaddr_in &clientAddr);
    void HandleJoin(const sockaddr_in &addr);
    void HandleDisconnect(const char *buffer, const sockaddr_in &clientAddr);
    bool AddressesEqual(const sockaddr_in &a, const sockaddr_in &b);
    network::ClientConnection *FindClient(const sockaddr_in &addr);

  private:
    static constexpr int MAX_PLAYERS = 4;
    int m_nextPlayerId = 1;
    bool m_running = false;
    std::array<network::ClientConnection, MAX_PLAYERS> m_clients;
    network::Server m_server;
    GameSimulation m_simulation;
};
