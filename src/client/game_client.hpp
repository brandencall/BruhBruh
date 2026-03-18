#pragma once
#include "../client/client_transport.hpp"
#include "../network/packet.hpp"
#include "../state/world_state.hpp"
#include "characters/character_types.hpp"
#include "client_bullet_system.hpp"
#include "map/map_types.hpp"
#include "map/wall_manager.hpp"
#include "renderers/character_renderer.hpp"
#include "ui/ui_manager.hpp"
#include <cstddef>
#include <cstdint>
#include <functional>
#include <unordered_map>

struct PredictedBullet {
    int localSlot;
};

using PacketHandler = std::function<void(const char *buffer)>;

class GameClient {
  public:
    GameClient() = default;
    ~GameClient();
    void Initialize();
    void Start(const char *ip, int port);

  private:
    void RegisterHandlers();

    void Disconnect();
    void Connect(const char *ip, int port);
    void SendJoin();
    void Update();
    void Sync(float dt);
    void Render();

    // TODO: Make a renderer instead of having the game client handle this
    void DrawMap(const Map::MapData &map);

    void Receive();
    void HandlePacket(char *buffer, size_t size);
    void HandleJoinResponse(const char *buffer);
    void HandleStateResponse(const char *buffer);
    // Handle bullets events
    void HandleBulletSpawn(const char *buffer);
    void HandleBulletHit(const char *buffer);
    void HandleBulletExpired(const char *buffer);
    // Handle player events
    void HandlePlayerDied(const char *buffer);
    // Handle wall events
    void HandlePlaceWall(const char *buffer);
    void HandleDestroyWall(const char *buffer);

    network::InputPacket CollectInput();
    void SendInput(network::InputPacket &packet);

    void SetGameRunning(bool runningState);
    void DrawDebugGrid();

  private:
    static constexpr float m_sendInterval = 1.0f / 30.0f;
    float m_sendAccumulator = 0.0f;
    float m_joinRetryAccumulator = 0.0f;
    Character::CharacterId m_characterId = Character::CharacterId::None;
    bool m_joined = false;
    bool m_running = false;
    uint32_t m_inputSequence = 0;
    uint8_t m_lastButtons = 0;

    std::unordered_map<network::PacketType, PacketHandler> m_handlers;
    network::ClientTransport m_transport;
    ClientWorldState m_worldState;
    Camera2D m_camera;
    System::ClientBulletSystem m_bulletSystem;
    Map::WallManager m_wallManager{false};
    CharacterRenderer m_characterRender;
    UI::UIManager m_ui;
};
