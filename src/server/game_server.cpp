#include "game_server.hpp"
#include <chrono>

GameServer::GameServer()
    : m_broadcaster(m_transport, m_registry, m_tick),
      m_packetHandler(m_transport, m_registry, m_simulation, m_broadcaster) {}

void GameServer::Start(int port) { m_running = m_transport.start(static_cast<uint16_t>(port)); }

bool GameServer::IsRunning() { return m_running; }

void GameServer::RunServer() {
    const float tickRate = 1.0f / 30.0f;
    float accumulator = 0.0f;

    auto previousTime = std::chrono::steady_clock::now();
    m_simulation.Initialize(m_eventBus);

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
