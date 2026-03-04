#include "game_server.hpp"
#include "game_simulation.hpp"
#include <chrono>
#include <cstddef>
#include <iostream>

void GameServer::Start(int port) { m_running = m_transport.start(static_cast<uint16_t>(port)); }

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
    network::InboundPacket pkt;
    while (m_transport.recv(pkt)) { // drain all queued packets
        HandlePacket(pkt.data, pkt.size, pkt.from);
    }
}

void GameServer::BroadcastState() {
    for (const auto &client : m_clients) {
        if (client.active) {
            SendFullSnapshot(client.peerId); // ← peerId not address
        }
    }
}

void GameServer::HandlePacket(char *buffer, size_t bytes, network::PeerId from) {
    auto *header = reinterpret_cast<network::PacketHeader *>(buffer);

    switch (header->type) {
    case network::PacketType::Join:
        HandleJoin(from);
        break;
    case network::PacketType::Input:
        HandleInput(buffer, bytes, from);
        break;
    case network::PacketType::Disconnect:
        HandleDisconnect(buffer, from);
        break;
    default:
        break;
    }
}

void GameServer::HandleJoin(network::PeerId from) {
    if (FindClientByPeer(from))
        return; // already registered

    for (size_t i = 0; i < m_clients.size(); i++) {
        if (!m_clients[i].active) {
            m_clients[i].active = true;
            m_clients[i].peerId = from; // ← store PeerId
            m_clients[i].playerId = i;

            network::JoinResponsePacket response{};
            response.header.type = network::PacketType::JoinResponse;
            response.playerId = m_clients[i].playerId;
            m_transport.send(from, &response, sizeof(response)); // ← transport.send

            m_simulation.CreatePlayer(m_clients[i].playerId);
            SendFullSnapshot(from);
            return;
        }
    }

    std::cout << "Server full\n";
}

void GameServer::SendFullSnapshot(network::PeerId peerId) {
    BuildStatePacket();
    size_t sendSize = offsetof(network::StatePacket, bullets) + m_statePacket.bulletCount * sizeof(state::BulletState);
    m_transport.send(peerId, &m_statePacket, sendSize);
}

void GameServer::HandleInput(char *buffer, size_t size, network::PeerId from) {
    if (size < sizeof(network::InputPacket))
        return;

    auto *packet = reinterpret_cast<network::InputPacket *>(buffer);
    auto *client = FindClientByPeer(from);
    if (!client)
        return;
    if (packet->playerId != client->playerId)
        return;

    m_simulation.ApplyInput(client->playerId, {
                                                  .moveX = packet->moveX,
                                                  .moveY = packet->moveY,
                                                  .aimX = packet->aimX,
                                                  .aimY = packet->aimY,
                                                  .buttons = packet->buttons,
                                              });
}

void GameServer::BuildStatePacket() {
    m_statePacket.header.type = network::PacketType::State;
    m_statePacket.tick = m_tick++;
    m_statePacket.playerCount = 0;
    auto players = m_simulation.GetPlayers();

    for (int i = 0; i < MAX_PLAYERS; ++i) {
        if (players[i].active) {
            m_statePacket.playerCount++;
            const state::PlayerState &p = players[i];
            m_statePacket.players[i].id = p.id;
            m_statePacket.players[i].position.x = p.position.x;
            m_statePacket.players[i].position.y = p.position.y;
            m_statePacket.players[i].health = p.health;
            m_statePacket.players[i].hurtbox = p.hurtbox;
            m_statePacket.players[i].active = p.active ? 1 : 0;
        }
    }

    const auto &bullets = m_simulation.GetBullets();
    m_statePacket.bulletCount = 0;
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].active)
            m_statePacket.bullets[m_statePacket.bulletCount++] = bullets[i];
    }
}

network::ClientConnection *GameServer::FindClientByPeer(network::PeerId peerId) {
    for (auto &client : m_clients) {
        if (client.active && client.peerId == peerId)
            return &client;
    }
    return nullptr;
}

void GameServer::HandleDisconnect(char *buffer, network::PeerId from) {
    auto *client = FindClientByPeer(from);
    if (!client)
        return;

    m_simulation.RemovePlayer(client->playerId);
    client->active = false;
}
