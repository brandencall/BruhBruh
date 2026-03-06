#pragma once
// TODO: move the ClientConnection out of here
#include "../network/client.hpp"
#include "../network/packet.hpp"
#include "../server/server_transport.hpp"
#include "event_bus.hpp"
#include "game_simulation.hpp"
#include <array>

class GameServer {
  public:
    GameServer() = default;
    void Start(int port);
    void RunServer();
    bool IsRunning();

  private:
    void UpdateSimulation(float dt);
    void Receive();
    void BroadcastAll(const void *data, size_t size);
    void BroadcastState();
    void BuildStatePacket();
    void SendFullSnapshot(network::PeerId peerId); // ← PeerId not ClientConnection

    void HandlePacket(char *buffer, size_t bytes, network::PeerId from); // ← PeerId
    void HandleJoin(network::PeerId from);
    void HandleDisconnect(char *buffer, network::PeerId from);
    void HandleInput(char *buffer, size_t bytes, network::PeerId from);

    network::ClientConnection *FindClientByPeer(network::PeerId peerId); // ← by PeerId

  private:
    bool m_running = false;
    int m_tick = 0;
    std::array<network::ClientConnection, MAX_PLAYERS> m_clients;
    network::ServerTransport m_transport; // ← replaces m_server
    EventBus m_eventBus;
    GameSimulation m_simulation;
    network::StatePacket m_statePacket{};
};
