#pragma once
#include "../../config.hpp"
#include "../../shared/network/ITransport.hpp"
#include "../../shared/state/lobby_slot_state.hpp"
#include "../event_hub.hpp"
#include "../network/network_message_handler.hpp"
#include "../session_manager.hpp"
#include "../ui/ui_manager.hpp"
#include "scene.hpp"
#include <array>
#include <map>

class LobbyScene : public Scene {
  public:
    LobbyScene(network::ITransport &transport, NetworkMessageHandler &handler, SessionManager &sessionManager);

    void OnEnter() override;
    void OnExit() override;
    void Update(float dt) override;
    void Render() override;

  private:
    void UpdateCharacterSelection(Vector2 mousePos,
                                  const std::unordered_map<Character::CharacterId, uint32_t> &takenCharacters);
    void UpdateInviteButton(Vector2 mousePos);

    // Packet handlers
    void HandleJoinResponse(const char *buf);
    void HandlePlayerJoined(const char *buf);
    void HandlePlayerReady(const char *buf);
    void HandleCharacterSelected(const char *buf);
    void HandleLobbyState(const char *buf);
    void HandleGameStarting(const char *buf);
    void HandleGameBegin(const char *buf);
    void HandleHostDisconnected(const char *buf);

    void SendJoin();
    void FlipReadyState();

    void InviteFriendsButton(int screenW, int screenH, bool mouseClicked, Vector2 mousePos);

    void OnCharacterSelected(const Character::CharacterId &character);

    void RenderCharacterIcons(const std::unordered_map<Character::CharacterId, uint32_t> &takenCharacters,
                              Vector2 mousePos);

    void RenderInviteButton(int screenW, int screenH, Vector2 mousePos);
    void RenderPlayerSlot(int slot, const state::LobbySlotState &player, int x, int y, int screenW, int screenH);
    void RenderSelectedCharacter(const state::LobbySlotState &player, int slotW, int slotH, int x, int y);

  private:
    bool m_joined = false;
    float m_joinRetryAccumulator = 0.0f;
    float m_countdownTimer = 0.0f;
    bool m_gameStarting = false;
    network::ITransport &m_transport;
    NetworkMessageHandler &m_handler;
    SessionManager &m_sessionManager;
    UI::UIManager m_ui;

    std::array<state::LobbySlotState, MAX_PLAYERS> m_players{};
    std::map<Character::CharacterId, Texture2D> m_icons;
    int m_localPlayerId = -1;
    bool m_ready = false;
};
