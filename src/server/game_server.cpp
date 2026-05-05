#include "game_server.hpp"
#include "../network/packets/lobby_packets.hpp"
#include "network/ITransport.hpp"
#include "network/steam_lobby_manager.hpp"
#include "server_phase.hpp"
#include <chrono>
#include <iostream>
#include <thread>

GameServer::GameServer()
    : m_broadcaster(m_registry, m_tick), m_packetHandler(m_registry, m_simulation, m_broadcaster, m_phase, m_lobby) {}

void GameServer::Start(int port) {
    m_running = m_ownedTransport.start(static_cast<uint16_t>(port));
    // TODO: NEED TO FIX THIS
    // m_transport = &m_ownedTransport;
    m_broadcaster.SetTransport(*m_transport);
}

void GameServer::Stop() {
    m_broadcaster.BroadcastHostDisconnected(m_hostPeerId);
    m_running.store(false);
}

// Steam path — transport is provided externally
void GameServer::StartInProcess(network::ITransport &transport, SteamLobbyManager &steamLobbyManager) {
    m_transport = &transport;
    m_broadcaster.SetTransport(transport);
    m_packetHandler.SetTransport(transport);
    m_steamLobbyManager = &steamLobbyManager;
    m_running = true;
}

void GameServer::SignalReady() { m_readyToRun.store(true); }

void GameServer::AddHostToLobby(std::string name) {
    m_hostPeerId = m_steamLobbyManager->AddHostToLobby();
    auto *client = m_registry.AddClient(m_hostPeerId);
    int slot = m_lobby.AddPlayer(m_hostPeerId, name.c_str(), client->playerId);
    std::cout << "Host added to lobby! Name: " << name << ", PeerId: " << m_hostPeerId
              << ", PlayerId: " << client->playerId << std::endl;
    network::JoinResponsePacket response{};
    response.header.type = network::PacketType::JoinResponse;
    response.playerId = client->playerId;
    response.characterId = Character::CharacterId::None;
    strncpy(response.name, name.c_str(), sizeof(response.name) - 1);
    m_transport->send(m_hostPeerId, &response, sizeof(response));
}

bool GameServer::IsRunning() { return m_running; }

void GameServer::RunServer() {
    while (!m_readyToRun)
        std::this_thread::sleep_for(std::chrono::milliseconds(16));

    m_simulation.Initialize(m_eventBus);

    while (m_running) {
        if (!m_transport)
            break;

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
        case ServerPhase::POSTGAME:
            TickPostGame();
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
        m_gameBeginTimer = 5.0f;
        m_gameRunning = true;
        m_broadcaster.BroadcastGameBegin(m_gameBeginTimer, m_simulation);
    }
}

void GameServer::SpawnPlayersIntoSimulation() {
    for (auto &slot : m_lobby.Slots()) {
        if (!slot.lobbySlot.occupied)
            continue;

        auto *client = m_registry.FindByPeer(slot.peerId);
        m_simulation.CreatePlayer(slot.lobbySlot.id, slot.lobbySlot.characterId, slot.lobbySlot.name);
    }
}

void GameServer::TickGameplay() {
    const float tickRate = 1.0f / 30.0f;
    float accumulator = 0.0f;

    auto previousTime = std::chrono::steady_clock::now();

    while (m_gameRunning) {
        auto now = std::chrono::steady_clock::now();
        float dt = std::chrono::duration<float>(now - previousTime).count();
        previousTime = now;
        dt = std::min(dt, 0.25f);

        Receive();

        if (m_gameBeginTimer > 0.0) {
            m_gameBeginTimer -= dt;
            m_broadcaster.BroadcastGameBegin(m_gameBeginTimer, m_simulation);
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
            continue;
        }

        accumulator += dt;

        while (accumulator >= tickRate) {
            UpdateSimulation(tickRate);

            if (m_simulation.GetGameTime() <= 0.0f) {
                m_phase = ServerPhase::POSTGAME;
                m_gameEndTimer = 5.0f;
                m_broadcaster.BroadcastGameEnd(m_gameEndTimer, m_simulation);
                m_gameRunning = false;
                break;
            }

            m_broadcaster.BroadcastState(m_simulation);
            accumulator -= tickRate;
        }
    }
}

void GameServer::TickPostGame() {
    std::this_thread::sleep_for(std::chrono::milliseconds(16));
    Receive();

    m_gameEndTimer -= 0.016f;
    m_broadcaster.BroadcastGameEnd(m_gameEndTimer, m_simulation);

    // Switch to lobby
    if (m_gameEndTimer <= 0.0f) {
        m_lobby.ResetLobbyState();
        m_simulation.Reset();
        m_phase = ServerPhase::LOBBY;
        m_broadcaster.BroadcastSwitchToLobby();
    }
}

void GameServer::UpdateSimulation(float tickRate) {
    m_simulation.Update(tickRate);
    m_broadcaster.DrainAndBroadcast(m_eventBus);
    m_eventBus.clear();
}

void GameServer::Receive() {
    network::InboundPacket pkt;
    while (m_running && m_transport->recvServer(pkt)) {
        m_packetHandler.Handle(pkt.data, pkt.size, pkt.from);
    }
}
