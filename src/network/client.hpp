#pragma once

#include <netinet/in.h>

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
    int m_socket = -1;
    uint32_t m_playerId = 0;
    sockaddr_in m_serverAddr{};
};

} // namespace network
