#include "server.hpp"
#include <cstring>
#include <iostream>
#include <sys/types.h>

namespace network {

bool Server::Start(uint16_t port) {
    if (!Net::Init()) {
        std::cerr << "Socket init failed\n";
        return false;
    }
    m_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (m_socket < 0) {
        std::cout << "Failed to create socket\n";
        return false;
    }

    std::memset(&m_addr, 0, sizeof(m_addr));
    m_addr.sin_family = AF_INET;
    m_addr.sin_addr.s_addr = INADDR_ANY;
    m_addr.sin_port = htons(port);

    if (bind(m_socket, (sockaddr *)&m_addr, sizeof(m_addr)) < 0) {
        std::cout << "Bind failed\n";
        Net::Close(m_socket);
        return false;
    }
    Net::SetNonBlocking(m_socket);
    std::cout << "Server started on port " << port << "\n";
    return true;
}

void Server::Stop() {
    if (m_socket >= 0)
        Net::Shutdown();
}

void Server::Send(const char *data, size_t size, const sockaddr_in &clientAddr) {
    sendto(m_socket, data, static_cast<int>(size), 0, reinterpret_cast<const sockaddr *>(&clientAddr),
           sizeof(clientAddr));
}

int Server::Receive(char *buffer, size_t size, sockaddr_in &outAddr) {
    socklen_t addrLen = sizeof(outAddr);

    int bytes = recvfrom(m_socket, buffer, size, 0, (sockaddr *)&outAddr, &addrLen);

    if (bytes <= 0)
        return 0;

    return bytes;
}

} // namespace network
