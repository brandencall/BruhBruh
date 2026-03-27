#pragma once
#include "../server/server_transport.hpp"
#include "event_bus.hpp"
#include "game_simulation.hpp"
#include "network/client_registry.hpp"
#include "network/packet_handler.hpp"
#include "network/state_broadcaster.hpp"

class GameServer {
  public:
    GameServer();
    void Start(int port);
    void RunServer();
    bool IsRunning();

  private:
    void UpdateSimulation(float tickRate);
    void Receive();

    bool m_running = false;
    int m_tick = 0;

    network::ServerTransport m_transport;
    network::ClientRegistry m_registry;
    EventBus m_eventBus;
    GameSimulation m_simulation;
    network::StateBroadcaster m_broadcaster;
    network::PacketHandler m_packetHandler;
};
