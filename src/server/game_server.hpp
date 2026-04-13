#pragma once
#include "../server/server_transport.hpp"
#include "event_bus.hpp"
#include "game_simulation.hpp"
#include "network/client_registry.hpp"
#include "network/packet_handler.hpp"
#include "network/state_broadcaster.hpp"
#include "server_lobby.hpp"
#include "server_phase.hpp"

class GameServer {
  public:
    GameServer();
    void Start(int port);
    void RunServer();
    bool IsRunning();

  private:
    void TickLobby();
    // May want to move this to broadcaster
    // void BroadcastStartGame();
    void TickStarting();
    void SpawnPlayersIntoSimulation();
    // May want to move this to broadcaster
    // void BroadcastCountdown(float startTimer);
    void TickGameplay();
    void UpdateSimulation(float tickRate);
    void Receive();

  private:
    ServerPhase m_phase = ServerPhase::LOBBY;
    bool m_running = false;
    int m_tick = 0;
    float m_startTimer = 0.0f;
    float m_gameBeginTimer = 0.0f;

    network::ServerTransport m_transport;
    network::ClientRegistry m_registry;
    EventBus m_eventBus;
    GameSimulation m_simulation;
    network::StateBroadcaster m_broadcaster;
    ServerLobby m_lobby;
    network::PacketHandler m_packetHandler;
};
