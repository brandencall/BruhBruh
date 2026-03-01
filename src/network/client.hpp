#pragma once

#include "platform_sockets.hpp"
#include <cstdint>

namespace network {

struct ClientConnection {
    sockaddr_in address;
    uint32_t playerId;
    bool active = false;
};

class Client {

  public:
    void Connect(const char *, int port);
    void Disconnect();

    void SendJoin();
    void Send(const char *data, size_t size);
    void Receive();
    void SendInput();

    bool IsRunning() const;
    void Update();

  private:
    bool m_running = false;
    Socket m_socket = INVALID_SOCKET;
    uint32_t m_playerId = 0;
    sockaddr_in m_serverAddr{};
};

} // namespace network
