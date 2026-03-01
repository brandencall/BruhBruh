#include "client.hpp"
#include "packet.hpp"
#include <cstring>
#include <iostream>
#include <thread>

namespace network {

void Client::Connect(const char *ip, int port) {
    m_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (m_socket < 0) {
        std::cout << "Failed to create socket\n";
        m_running = false;
        return;
    }

    std::memset(&m_serverAddr, 0, sizeof(m_serverAddr));
    m_serverAddr.sin_family = AF_INET;
    m_serverAddr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip, &m_serverAddr.sin_addr) <= 0) {
        std::cout << "Invalid address\n";
        m_running = false;
        return;
    }

    // "Connect" UDP socket (sets default send/recv target)
    if (connect(m_socket, (sockaddr *)&m_serverAddr, sizeof(m_serverAddr)) < 0) {
        std::cout << "UDP connect failed\n";
        close(m_socket);
        m_running = false;
        return;
    }

    fcntl(m_socket, F_SETFL, O_NONBLOCK);

    std::cout << "Connected to server " << ip << ":" << port << "\n";
    m_running = true;
}

void Client::Disconnect() {
    m_running = false;
    if (m_socket >= 0)
        Net::Shutdown();
}

void Client::SendJoin() {
    JoinPacket packet{};
    packet.header.type = PacketType::Join;

    send(m_socket, &packet, sizeof(packet), 0);
    Receive();
}

void Client::Send(const char *data, size_t size) { send(m_socket, data, size, 0); }

void Client::Receive() {
    char buffer[1024];

    ssize_t bytes = recv(m_socket, buffer, sizeof(buffer), 0);
    if (bytes <= 0)
        return;

    PacketHeader *header = (PacketHeader *)buffer;

    if (header->type == PacketType::JoinResponse) {
        JoinResponsePacket *response = (JoinResponsePacket *)buffer;
        m_playerId = response->playerId;
        std::cout << "Assigned Player ID: " << response->playerId << "\n";
    }
}

bool Client::IsRunning() const { return m_running; };

void Client::SendInput() {
    InputPacket packet{};
    packet.header.type = PacketType::Input;
    packet.playerId = m_playerId;

    send(m_socket, &packet, sizeof(packet), 0);
}

void Client::Update() {
    Receive();
    SendInput();
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
}

} // namespace network
