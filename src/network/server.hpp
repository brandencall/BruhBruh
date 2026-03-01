#pragma once

#include "client.hpp"
#include <array>
#include <cstdint>

namespace network {

class Server {

  public:
    void Start(uint16_t port);
    void Stop();

    void Receive();
    void Send(const char *data, size_t size, const sockaddr_in &clientAddr);
    void HandleJoin(const sockaddr_in &clientAddr);
    void HandleInput(char *buffer, int bytes, sockaddr_in &addr);

    bool IsRunning() const;
    void Tick() const;

  private:
    ClientConnection *FindClient(const sockaddr_in &addr);

  private:
    static constexpr int MAX_PLAYERS = 4;

    std::array<ClientConnection, MAX_PLAYERS> m_clients;
    uint32_t m_nextPlayerId = 1;
    bool m_running = false;
    int m_socket = -1;
    sockaddr_in m_addr{};
};

} // namespace network
