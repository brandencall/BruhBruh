#pragma once

#include "../shared/network/ITransport.hpp"
#include "platform_sockets.hpp"
#include <cstdint>

namespace network {

struct ClientConnection {
    network::PeerId peerId;
    uint32_t playerId;
    bool active = false;
};

class Client {

  public:
    bool Connect(const char *, int port);
    void Disconnect();
    void Send(const char *data, size_t size);
    int Receive(char *buffer, size_t size);

  private:
    Socket m_socket = INVALID_SOCKET;
    sockaddr_in m_serverAddr{};
};

} // namespace network
