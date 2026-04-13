#include "game_server.hpp"
#include <chrono>
#include <iostream>
#include <thread>

GameServer::GameServer()
    : m_broadcaster(m_transport, m_registry, m_tick),
      m_packetHandler(m_transport, m_registry, m_simulation, m_broadcaster, m_phase, m_lobby) {}

void GameServer::Start(int port) { m_running = m_transport.start(static_cast<uint16_t>(port)); }

bool GameServer::IsRunning() { return m_running; }

void GameServer::RunServer() {
    m_simulation.Initialize(m_eventBus);

    while (m_running) {
        switch (m_phase) {
        case ServerPhase::LOBBY:
            TickLobby();
            break;
        case ServerPhase::STARTING:
            TickStarting();
            break;
        case ServerPhase::GAMEPLAY:
            TickGameplay();
            break;
        case ServerPhase::ENDED:
            m_running = false;
            break;
        }
    }
}

void GameServer::TickLobby() {
    std::this_thread::sleep_for(std::chrono::milliseconds(16));

    Receive();
    // Broadcast the lobby state
    m_broadcaster.BroadcastLobbyState(m_lobby);

    if (m_lobby.AllReady() && m_lobby.PlayerCount() >= 2) {
        m_phase = ServerPhase::STARTING;
        m_startTimer = 3.0f; // 3 second countdown
        m_broadcaster.BroadcastStartGame(m_startTimer);
    }
}

void GameServer::TickStarting() {
    std::this_thread::sleep_for(std::chrono::milliseconds(16));
    Receive();

    m_startTimer -= 0.016f;
    m_broadcaster.BroadcastStartGame(m_startTimer);

    if (m_startTimer <= 0.0f) {
        SpawnPlayersIntoSimulation();
        m_phase = ServerPhase::GAMEPLAY;
        m_broadcaster.BroadcastGameBegin();
    }
}

void GameServer::SpawnPlayersIntoSimulation() {
    for (auto &slot : m_lobby.Slots()) {
        if (!slot.lobbySlot.occupied)
            continue;

        auto *client = m_registry.FindByPeer(slot.peerId);
        m_simulation.CreatePlayer(client->playerId, client->characterId);
    }
}

void GameServer::TickGameplay() {
    const float tickRate = 1.0f / 30.0f;
    float accumulator = 0.0f;

    auto previousTime = std::chrono::steady_clock::now();

    while (m_running) {
        auto now = std::chrono::steady_clock::now();
        float dt = std::chrono::duration<float>(now - previousTime).count();
        previousTime = now;

        accumulator += dt;

        Receive();

        while (accumulator >= tickRate) {
            UpdateSimulation(tickRate);
            m_broadcaster.BroadcastState(m_simulation);
            accumulator -= tickRate;
        }
    }
}

void GameServer::UpdateSimulation(float tickRate) {
    m_simulation.Update(tickRate);
    m_broadcaster.DrainAndBroadcast(m_eventBus);
    m_eventBus.clear();
}

void GameServer::Receive() {
    network::InboundPacket pkt;
    while (m_transport.recv(pkt))
        m_packetHandler.Handle(pkt.data, pkt.size, pkt.from);
}
