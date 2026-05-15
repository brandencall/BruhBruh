#pragma once
#include "../../network/packets/gameplay_packets.hpp"
#include "../../shared/network/ITransport.hpp"
#include "../../shared/state/lobby_slot_state.hpp"
#include "../event_hub.hpp"
#include "../network/network_message_handler.hpp"
#include "../renderers/character_renderer.hpp"
#include "../renderers/tilemap_renderer.hpp"
#include "../renderers/wall_renderer.hpp"
#include "../session_manager.hpp"
#include "../state/world_state.hpp"
#include "../systems/audio_system.hpp"
#include "../systems/character_camera.hpp"
#include "../systems/client_bullet_system.hpp"
#include "../ui/ui_manager.hpp"
#include "raylib.h"
#include "scene.hpp"
#include <cstdint>

struct PendingInput {
    network::InputPacket packet;
    float dt;
};

class GameScene : public Scene {
  public:
    GameScene(network::ITransport &transport, NetworkMessageHandler &handler, SessionManager &sessionManager,
              state::LobbySlotState currentPlayerState);

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
    void HandleWallPickedUp(const char *buf);
    void HandleGameEnd(const char *buf);
    void HandleSwitchToLobby(const char *buf);
    void HandleHostDisconnected(const char *buf);

    void DrawMap(const Map::MapData &map);
    void RenderConnecting();
    void RenderCursor();

    void TickPrediction(float dt);
    void PredictLocalActions();
    network::InputPacket BuildInputPacket();

    void Reconcile(Vector2 serverPos, uint32_t ackedSeq);

  private:
    Client::EventHub m_events;
    network::ITransport &m_transport;
    NetworkMessageHandler &m_handler;
    SessionManager &m_sessionManager;
    uint32_t m_currentPlayerId = UINT32_MAX;
    Character::CharacterId m_currenCharacterId = Character::CharacterId::None;

    // Scene-owned state
    ClientWorldState m_worldState;
    System::ClientBulletSystem m_bulletSystem;
    Map::WallManager m_wallManager;
    Render::TilemapRenderer m_tilemapRenderer;
    Render::CharacterRenderer m_characterRender;
    Render::WallRenderer m_wallRender;
    UI::UIManager m_ui;
    System::AudioSystem m_audioSystem;

    static constexpr size_t INPUT_BUFFER_SIZE = 128;
    static constexpr float SNAP_THRESHOLD = 64.0f; // pixels — tune to your tile size
    std::array<PendingInput, INPUT_BUFFER_SIZE> m_inputBuffer{};
    uint32_t m_lastAckedSeq = 0;
    Vector2 m_predictedPos{0.0f, 0.0f};
    Vector2 m_smoothedPredictedPos{0.0f, 0.0f};
    bool m_predictionInitialised = false;

    uint16_t m_inputSequence = 1;
    uint8_t m_lastButtons = 0;
    uint32_t m_localBulletSeq = 0;
    float m_sendAccumulator = 0.0f;
    float m_sendInterval = 1.0f / 60.0f;

    bool m_joined = false;
    float m_gameBeginTimer = 0.0f;
    bool m_initialSnapDone = false;
    bool m_gameEndScreenActive = false;
    bool m_audioAvailable = false;

    System::CharacterCamera m_camera{};
};
