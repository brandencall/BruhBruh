#include "client.hpp"
#include "platform_sockets.hpp"
#include <cstring>
#include <iostream>

namespace network {

bool Client::Connect(const char *ip, int port) {
    m_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (m_socket < 0) {
        std::cout << "Failed to create socket\n";
        return false;
    }

    std::memset(&m_serverAddr, 0, sizeof(m_serverAddr));
    m_serverAddr.sin_family = AF_INET;
    m_serverAddr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip, &m_serverAddr.sin_addr) <= 0) {
        std::cout << "Invalid address\n";
        return false;
    }

    // "Connect" UDP socket (sets default send/recv target)
    if (connect(m_socket, (sockaddr *)&m_serverAddr, sizeof(m_serverAddr)) < 0) {
        std::cout << "UDP connect failed\n";
        Net::Close(m_socket);
        return false;
    }

    Net::SetNonBlocking(m_socket);

    std::cout << "Connected to server " << ip << ":" << port << "\n";
    return true;
}

void Client::Disconnect() {
    if (m_socket >= 0)
        Net::Shutdown();
}

void Client::Send(const char *data, size_t size) { send(m_socket, data, static_cast<int>(size), 0); }

int Client::Receive(char *buffer, size_t size) {
    int bytes = recv(m_socket, buffer, size, 0);
    if (bytes <= 0)
        return 0;

    return bytes;
}

} // namespace network
