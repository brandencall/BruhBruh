#pragma once
#include "../client/client_transport.hpp"
#include "../network/packet.hpp"
#include "../state/world_state.hpp"
#include "client_bullet_system.hpp"
#include "map/map_types.hpp"
#include <cstddef>
#include <cstdint>

struct PredictedBullet {
    int localSlot;
};

class GameClient {
  public:
    GameClient() = default;
    ~GameClient();
    void Initialize();
    void Start(const char *ip, int port);

  private:
    void Disconnect();
    void Connect(const char *ip, int port);
    void SendJoin();
    void Update();
    void Sync(float dt);
    void Render();

    // TODO: Make a renderer instead of having the game client handle this
    void DrawMap(const MapData &map);

    void Receive();
    void HandlePacket(char *buffer, size_t size);
    void HandleJoinResponse(const char *buffer);
    void HandleStateResponse(const char *buffer, size_t size);
    void HandleBulletSpawn(const char *buffer);
    void HandleBulletHit(const char *buffer);
    void HandleBulletExpired(const char *buffer);

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
