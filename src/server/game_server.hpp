#pragma once
#include "../server/server_transport.hpp"
#include "../shared/network/ITransport.hpp"
#include "../shared/network/steam_lobby_manager.hpp"
#include "event_bus.hpp"
#include "game_simulation.hpp"
#include "network/client_registry.hpp"
#include "network/packet_handler.hpp"
#include "network/state_broadcaster.hpp"
#include "server_lobby.hpp"
#include "server_phase.hpp"
#include <atomic>

class GameServer {
  public:
    GameServer();
    void Start(int port);
    void Stop();
    void SignalReady();
    void AddHostToLobby(std::string name);
    void StartInProcess(network::ITransport &transport, SteamLobbyManager &steamLobbyManager);
    void RunServer();
    bool IsRunning();

  private:
    void TickLobby();
    void TickStarting();
    void SpawnPlayersIntoSimulation();
    void TickGameplay();
    void TickPostGame();
    void UpdateSimulation(float tickRate);
    void Receive();

  private:
    ServerPhase m_phase = ServerPhase::LOBBY;
    std::atomic<bool> m_running = false;
    std::atomic<bool> m_readyToRun = false;
    bool m_gameRunning = false;
    int m_tick = 0;
    float m_startTimer = 0.0f;
    float m_gameBeginTimer = 0.0f;
    float m_gameEndTimer = 0.0f;

    network::ServerTransport m_ownedTransport;
    network::ITransport *m_transport = nullptr;
    SteamLobbyManager *m_steamLobbyManager = nullptr;
    network::ClientRegistry m_registry;
    EventBus m_eventBus;
    GameSimulation m_simulation;
    network::StateBroadcaster m_broadcaster;
    ServerLobby m_lobby;
    network::PacketHandler m_packetHandler;
    network::PeerId m_hostPeerId;
};
