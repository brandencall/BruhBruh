#pragma once
#include "../../config.hpp"
#include "../../shared/state/lobby_slot_state.hpp"
#include "../event_bus.hpp"
#include "../game.hpp"
#include "../ui/ui_manager.hpp"
#include "scene.hpp"
#include <array>
#include <map>

class LobbyScene : public Scene {
  public:
    LobbyScene(Game &game);

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
    void SendLocalJoin();
    void SendDisconnect();
    void FlipReadyState();

    void InviteFriendsButton(int screenW, int screenH, bool mouseClicked, Vector2 mousePos);

    void OnCharacterSelected(const Character::CharacterId &character);

    void RenderCharacterIcons(const std::unordered_map<Character::CharacterId, uint32_t> &takenCharacters,
                              Vector2 mousePos);

    void RenderInviteButton(int screenW, int screenH, Vector2 mousePos);
    void RenderPlayerSlot(int slot, const state::LobbySlotState &player, int x, int y, int screenW, int screenH);
    void RenderSelectedCharacter(const state::LobbySlotState &player, int slotW, int slotH, int x, int y);

  private:
    // ─────────────────────────────────────────────────────────────────────────────
    // Style constants  (mirrors main menu / pause menu palette)
    // ─────────────────────────────────────────────────────────────────────────────
    static constexpr Color kLobbyBg = {10, 10, 16, 255};
    static constexpr Color kLobbyAccent = {45, 80, 160, 255};
    static constexpr Color kLobbyAccentHover = {70, 120, 220, 255};
    static constexpr Color kLobbyBorder = {70, 70, 100, 255};
    static constexpr Color kLobbyDivider = {70, 70, 90, 255};
    static constexpr Color kLobbyTextPrimary = {255, 255, 255, 255};
    static constexpr Color kLobbyTextMuted = {160, 160, 180, 255};
    static constexpr Color kLobbySlotFill = {18, 18, 28, 255};
    static constexpr Color kLobbyReady = {60, 160, 80, 255};
    static constexpr Color kLobbyNotReady = {160, 55, 55, 255};

    Game &m_game;
    UI::UIManager m_ui;

    bool m_pendingJoin = false;
    bool m_joined = false;
    float m_joinRetryAccumulator = 0.0f;

    int m_countdownTimer = -1;
    int m_previousCountdown = -1;
    int m_maxCountdown = -1;

    bool m_gameStarting = false;

    std::array<state::LobbySlotState, MAX_PLAYERS> m_players{};
    std::map<Character::CharacterId, Texture2D> m_icons;
    int m_localPlayerId = -1;
    bool m_ready = false;

    Client::EventBus<client::GameStartingEvent> onGameStarting;
};
