#pragma once
#include "../../config.hpp"
#include "../../shared/state/lobby_slot_state.hpp"
#include "../client_transport.hpp"
#include "../event_hub.hpp"
#include "../network/network_message_handler.hpp"
#include "scene.hpp"
#include <array>
#include <map>

class LobbyScene : public Scene {
  public:
    LobbyScene(Client::EventHub &events, network::ClientTransport &transport, NetworkMessageHandler &handler,
               SceneManager &sceneManager);

    void OnEnter() override;
    void OnExit() override;
    void Update(float dt) override;
    void Render() override;

  private:
    // Packet handlers
    void HandleJoinResponse(const char *buf);
    void HandlePlayerJoined(const char *buf);
    void HandlePlayerReady(const char *buf);
    void HandleCharacterSelected(const char *buf);
    void HandleLobbyState(const char *buf);
    void HandleGameStarting(const char *buf);
    void HandleGameBegin(const char *buf);

    void SendJoin();
    void FlipReadyState();

    void OnCharacterSelected(const Character::CharacterId &character);

    void RenderPlayerSlot(int slot, const state::LobbySlotState &player, int x, int y, int screenW, int screenH);
    void RenderSelectedCharacter(const state::LobbySlotState &player, int slotW, int slotH, int x, int y);

  private:
    bool m_joined = false;
    float m_joinRetryAccumulator = 0.0f;
    float m_countdownTimer = 0.0f;
    bool m_gameStarting = false;
    Client::EventHub &m_events;
    network::ClientTransport &m_transport;
    NetworkMessageHandler &m_handler;
    SceneManager &m_sceneManager;

    std::array<state::LobbySlotState, MAX_PLAYERS> m_players{};
    std::map<Character::CharacterId, Texture2D> m_icons;
    int m_localPlayerId = -1;
    bool m_ready = false;
};
