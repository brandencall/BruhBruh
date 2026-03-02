#pragma once

#include "platform_sockets.hpp"

namespace network {

class Server {

  public:
    bool Start(uint16_t port);
    void Stop();
    void Send(const char *data, size_t size, const sockaddr_in &clientAddr);
    int Receive(char *buffer, size_t size, sockaddr_in &outAddr);

  private:
    int m_socket = -1;
    sockaddr_in m_addr{};
};

} // namespace network
