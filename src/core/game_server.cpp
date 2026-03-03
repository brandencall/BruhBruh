#include "game_server.hpp"
#include "game_simulation.hpp"
#include <chrono>
#include <cstddef>
#include <iostream>

void GameServer::Start(int port) { m_running = m_server.Start(port); }

bool GameServer::IsRunning() { return m_running; }

void GameServer::RunServer() {
    const float tickRate = 1.0f / 30.0f;
    float accumulator = 0.0f;

    auto previousTime = std::chrono::steady_clock::now();

    while (m_running) {
        auto now = std::chrono::steady_clock::now();
        float dt = std::chrono::duration<float>(now - previousTime).count();
        previousTime = now;

        accumulator += dt;

        // Always receive packets as fast as possible
        Receive();

        while (accumulator >= tickRate) {
            UpdateSimulation(tickRate);
            BroadcastState();

            accumulator -= tickRate;
        }
    }
}

void GameServer::UpdateSimulation(float tickRate) { m_simulation.Update(tickRate); }

void GameServer::Receive() {
    char buffer[1024];
    sockaddr_in clientAddr{};

    int bytes = m_server.Receive(buffer, sizeof(buffer), clientAddr);
    if (bytes <= 0)
        return;

    HandlePacket(buffer, bytes, clientAddr);
}

void GameServer::BroadcastState() {
    for (const auto client : m_clients) {
        if (client.active) {
            SendFullSnapshot(client);
        }
    }
}

void GameServer::HandlePacket(char *buffer, size_t bytes, sockaddr_in &clientAddr) {
    network::PacketHeader *header = (network::PacketHeader *)buffer;

    switch (header->type) {
    case network::PacketType::Join:
        HandleJoin(clientAddr);
        break;

    case network::PacketType::Input:
        HandleInput(buffer, bytes, clientAddr);
        break;
    case network::PacketType::Disconnect:
        HandleDisconnect(buffer, clientAddr);
        break;

    default:
        break;
    }
}

void GameServer::HandleJoin(const sockaddr_in &addr) {
    // Handle the join request from the client
    std::cout << "Client joined: " << inet_ntoa(addr.sin_addr) << ":" << ntohs(addr.sin_port) << std::endl;
    // Check if already joined
    if (FindClient(addr))
        return;

    for (size_t i = 0; i < m_clients.size(); i++) {
        if (!m_clients[i].active) {
            m_clients[i].active = true;
            m_clients[i].address = addr;
            m_clients[i].playerId = i;

            network::JoinResponsePacket response{};
            response.header.type = network::PacketType::JoinResponse;
            response.playerId = m_clients[i].playerId;

            m_server.Send(reinterpret_cast<const char *>(&response), static_cast<int>(sizeof(response)), addr);
            std::cout << "Player joined. ID: " << m_clients[i].playerId << "\n";
            m_simulation.CreatePlayer(m_clients[i].playerId);
            SendFullSnapshot(m_clients[i]);

            return;
        }
    }

    std::cout << "Server full\n";
}

void GameServer::SendFullSnapshot(network::ClientConnection client) {
    BuildStatePacket();

    // Ofset from the start of the StatePacket struct to base of the struct (bullets and bullet count)
    size_t sendSize = offsetof(network::StatePacket, bullets) + m_statePacket.bulletCount * sizeof(state::BulletState);
    m_server.Send(reinterpret_cast<const char *>(&m_statePacket), sendSize, client.address);
}

void GameServer::HandleInput(char *buffer, size_t size, const sockaddr_in &clientAddr) {
    if (size < sizeof(network::InputPacket))
        return;

    network::InputPacket *packet = (network::InputPacket *)buffer;

    network::ClientConnection *client = FindClient(clientAddr);
    if (!client)
        return;

    if (packet->playerId != client->playerId)
        return;

    state::PlayerInput input = state::PlayerInput{
        .moveX = packet->moveX,
        .moveY = packet->moveY,
        .aimX = packet->aimX,
        .aimY = packet->aimY,
        .buttons = packet->buttons,
    };

    m_simulation.ApplyInput(client->playerId, input);
}

void GameServer::BuildStatePacket() {
    m_statePacket.header.type = network::PacketType::State;
    m_statePacket.tick = m_tick++;

    const auto &players = m_simulation.GetActivePlayers();
    m_statePacket.playerCount = std::min<uint16_t>(players.size(), MAX_PLAYERS);

    for (int i = 0; i < m_statePacket.playerCount; i++) {
        const state::PlayerState &p = players[i];
        m_statePacket.players[i].id = p.id;
        m_statePacket.players[i].position.x = p.position.x;
        m_statePacket.players[i].position.y = p.position.y;
        m_statePacket.players[i].health = p.health;
        m_statePacket.players[i].active = p.active ? 1 : 0;
    }

    const auto &bullets = m_simulation.GetBullets();
    m_statePacket.bulletCount = 0;
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].active)
            m_statePacket.bullets[m_statePacket.bulletCount++] = bullets[i];
    }
}

network::ClientConnection *GameServer::FindClient(const sockaddr_in &addr) {
    for (auto &client : m_clients) {
        if (!client.active)
            continue;

        if (client.active && client.address.sin_addr.s_addr == addr.sin_addr.s_addr &&
            client.address.sin_port == addr.sin_port) {
            return &client;
        }
    }

    return nullptr;
}

void GameServer::HandleDisconnect(const char *buffer, const sockaddr_in &clientAddr) {
    auto *response = (network::DisconnectPacket *)buffer;
    if (response->playerId < 0 || response->playerId > MAX_PLAYERS)
        return;

    if (!AddressesEqual(clientAddr, m_clients[response->playerId].address))
        return;

    m_clients[response->playerId].active = false;
    m_simulation.RemovePlayer(response->playerId);
}

bool GameServer::AddressesEqual(const sockaddr_in &a, const sockaddr_in &b) {
    return a.sin_addr.s_addr == b.sin_addr.s_addr && a.sin_port == b.sin_port;
}
