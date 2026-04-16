#pragma once
#include "../../network/packets/gameplay_packets.hpp"
#include "../../shared/state/lobby_slot_state.hpp"
#include "../client_transport.hpp"
#include "../event_hub.hpp"
#include "../network/network_message_handler.hpp"
#include "../renderers/character_renderer.hpp"
#include "../renderers/tilemap_renderer.hpp"
#include "../state/world_state.hpp"
#include "../systems/client_bullet_system.hpp"
#include "../ui/ui_manager.hpp"
#include "raylib.h"
#include "scene.hpp"
#include "scene_manager.hpp"
#include <cstdint>

class GameScene : public Scene {
  public:
    GameScene(Client::EventHub &events, network::ClientTransport &transport, NetworkMessageHandler &handler,
              SceneManager &sceneManager, state::LobbySlotState currentPlayerState);

    void OnEnter() override;
    void OnExit() override;
    void Update(float dt) override;
    void Render() override;
    uint32_t GetCurrentPlayerId() const;

  private:
    void Sync(float dt);
    void HandleScoreboardInput();
    // Packet handlers — registered with NetworkMessageHandler
    void HandleGameBegin(const char *buf);
    void HandleStateResponse(const char *buf);
    void HandleCurrentWorldState(const char *buf);
    void HandleBulletSpawn(const char *buf);
    void HandleBulletDestroyed(const char *buf);
    void HandlePlayerRespawned(const char *buf);
    void HandlePlayerDamaged(const char *buf);
    void HandlePlayerDied(const char *buf);
    void HandlePlaceWall(const char *buf);
    void HandleWallDamaged(const char *buf);
    void HandleDestroyWall(const char *buf);
    void DrawMap(const Map::MapData &map);
    void RenderConnecting();

    network::InputPacket CollectInput();

  private:
    // Dependencies — all refs, owned by GameClient
    Client::EventHub &m_events;
    network::ClientTransport &m_transport;
    NetworkMessageHandler &m_handler;
    SceneManager &m_sceneManager;
    uint32_t m_currentPlayerId = UINT32_MAX;
    Character::CharacterId m_currenCharacterId = Character::CharacterId::None;

    // Scene-owned state
    ClientWorldState m_worldState;
    System::ClientBulletSystem m_bulletSystem;
    Map::WallManager m_wallManager;
    Render::TilemapRenderer m_tilemapRenderer;
    Render::CharacterRenderer m_characterRender;
    UI::UIManager m_ui;

    uint16_t m_inputSequence = 0;
    uint8_t m_lastButtons = 0;
    float m_sendAccumulator = 0.0f;
    float m_sendInterval = 1.0f / 60.0f;

    bool m_joined = false;
    float m_gameBeginTimer = 0.0f;
    bool m_cameraReady = false;
    bool m_initialSnapDone = false;

    Camera2D m_camera{};
};
