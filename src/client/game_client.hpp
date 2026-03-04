#pragma once
#include "../client/client_transport.hpp"
#include "../network/packet.hpp"
#include "../state/world_state.hpp"
#include "client_bullet_system.hpp"
#include <cstddef>
#include <cstdint>

class GameClient {
  public:
    GameClient();
    ~GameClient();
    void Connect(const char *ip, int port);
    void SendJoin();
    void Update();
    void Sync(float dt);
    bool GameRunning();

  private:
    void Disconnect();
    void Render();

    void Receive();
    void HandlePacket(char *buffer, size_t size);
    void HandleJoinResponse(const char *buffer);
    void HandleStateResponse(const char *buffer, size_t size);

    network::InputPacket CollectInput();
    void SendInput(network::InputPacket &packet);

    void SetGameRunning(bool runningState);
    void DrawDebugGrid();

  private:
    static constexpr float m_sendInterval = 1.0f / 30.0f;
    float m_sendAccumulator = 0.0f;
    int m_playerId = -1;
    bool m_joined = false;
    bool m_running = true;
    uint32_t m_inputSequence = 0;
    uint8_t m_lastButtons = 0;
    network::ClientTransport m_transport;
    ClientWorldState m_worldState;
    Camera2D m_camera;
    System::ClientBulletSystem m_bulletSystem;
};
