#include "game_server.hpp"
#include "../network/packet.hpp"
#include <cstddef>
#include <iostream>

GameServer::GameServer() : m_server(network::Server()) {}

void GameServer::Start(int port) { m_running = m_server.Start(port); }

bool GameServer::IsRunning() { return m_running; }

void GameServer::Receive() {
    char buffer[1024];
    sockaddr_in clientAddr{};

    int bytes = m_server.Receive(buffer, sizeof(buffer), clientAddr);
    if (bytes <= 0)
        return;

    HandlePacket(buffer, bytes, clientAddr);
}

void GameServer::HandlePacket(char *buffer, size_t bytes, sockaddr_in &clientAddr) {
    network::PacketHeader *header = (network::PacketHeader *)buffer;

    switch (header->type) {
    case network::PacketType::Join:
        HandleJoin(clientAddr);
        break;

    case network::PacketType::Input:
        // HandleInput(buffer, bytes, clientAddr);
        std::cout << "Received an Input packet type" << std::endl;
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

    // Find empty slot
    for (auto &client : m_clients) {
        if (!client.active) {
            client.active = true;
            client.address = addr;
            client.playerId = m_nextPlayerId++;

            network::JoinResponsePacket response{};
            response.header.type = network::PacketType::JoinResponse;
            response.playerId = client.playerId;

            m_server.Send(reinterpret_cast<const char *>(&response), static_cast<int>(sizeof(response)), addr);
            std::cout << "Player joined. ID: " << client.playerId << "\n";

            return;
        }
    }

    std::cout << "Server full\n";
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

// TODO: Actually use this
bool AddressesEqual(const sockaddr_in &a, const sockaddr_in &b) {
    return a.sin_addr.s_addr == b.sin_addr.s_addr && a.sin_port == b.sin_port;
}

void GameServer::BroadcastState(GameSimulation &simulation) {}
