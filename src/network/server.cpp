#include "server.hpp"
#include "packet.hpp"
#include <cstring>
#include <iostream>
#include <sys/types.h>
#include <thread>

namespace network {

void Server::Start(uint16_t port) {
    if (!Net::Init()) {
        std::cerr << "Socket init failed\n";
        m_running = false;
        return;
    }
    m_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (m_socket < 0) {
        std::cout << "Failed to create socket\n";
        m_running = false;
        return;
    }

    std::memset(&m_addr, 0, sizeof(m_addr));
    m_addr.sin_family = AF_INET;
    m_addr.sin_addr.s_addr = INADDR_ANY;
    m_addr.sin_port = htons(port);

    if (bind(m_socket, (sockaddr *)&m_addr, sizeof(m_addr)) < 0) {
        std::cout << "Bind failed\n";
        close(m_socket);
        m_running = false;
        return;
    }
    fcntl(m_socket, F_SETFL, O_NONBLOCK);
    std::cout << "Server started on port " << port << "\n";
    m_running = true;
}

void Server::Stop() {
    m_running = false;
    if (m_socket >= 0)
        Net::Shutdown();
}

void Server::HandleJoin(const sockaddr_in &addr) {
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

            JoinResponsePacket response{};
            response.header.type = PacketType::JoinResponse;
            response.playerId = client.playerId;

            sendto(m_socket, &response, sizeof(response), 0, (sockaddr *)&addr, sizeof(addr));

            std::cout << "Player joined. ID: " << client.playerId << "\n";

            return;
        }
    }

    std::cout << "Server full\n";
}

void Server::HandleInput(char *buffer, ssize_t bytes, sockaddr_in &addr) {
    ClientConnection *client = FindClient(addr);
    InputPacket *inputPacket = (InputPacket *)buffer;

    if (inputPacket->playerId != client->playerId) {
        std::cout << "Client spoofing playerId. Ignoring input.\n";
        return;
    }

    if (!client) {
        std::cout << "Received input from unknown address. Ignoring.\n";
        return; // discard
    }
    std::cout << "Received " << bytes << " bytes from client: " << inet_ntoa(addr.sin_addr) << ":"
              << ntohs(addr.sin_port) << std::endl;
    std::cout << "Data: " << buffer << std::endl;
}

void Server::Receive() {
    char buffer[1024];

    sockaddr_in clientAddr{};
    socklen_t addrLen = sizeof(clientAddr);

    ssize_t bytes = recvfrom(m_socket, buffer, sizeof(buffer), 0, (sockaddr *)&clientAddr, &addrLen);

    if (bytes <= 0)
        return;

    PacketHeader *header = (PacketHeader *)buffer;

    switch (header->type) {
    case PacketType::Join:
        HandleJoin(clientAddr);
        break;

    case PacketType::Input:
        HandleInput(buffer, bytes, clientAddr);
        break;

    default:
        break;
    }
}

void Server::Send(const char *data, size_t size, const sockaddr_in &clientAddr) {
    sendto(m_socket, data, size, 0, (sockaddr *)&clientAddr, sizeof(clientAddr));
}

ClientConnection *Server::FindClient(const sockaddr_in &addr) {
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

bool Server::IsRunning() const { return m_running; };

void Server::Tick() const {
    std::cout << "Tik Toc" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
}

} // namespace network
