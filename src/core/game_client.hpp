#pragma once
#include "../network/client.hpp"
#include "../network/packet.hpp"
#include <cstddef>

class GameClient {
  public:
    GameClient();
    ~GameClient();
    void Connect(const char *ip, int port);
    void SendJoin();
    // TODO: Implement
    void Update();
    bool GameRunning();

  private:
    void Render();

    void Receive();
    void HandlePacket(char *buffer, size_t size);

    network::InputPacket CollectInput();
    void SendInput(network::InputPacket &packet);
    // TODO: Remove when input is actually collected
    void SendInput();

    void SetGameRunning(bool runningState);

  private:
    int m_playerId = -1;
    bool m_joined = false;
    bool m_running = true;
    network::Client m_client;
};
