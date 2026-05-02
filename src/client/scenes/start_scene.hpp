#pragma once
#include "../event_hub.hpp"
#include "../game.hpp"
#include "../session_manager.hpp"
#include "../ui/ui_manager.hpp"
#include "scene.hpp"
#include "scene_manager.hpp"
#include <steam/steam_api.h>
#include <string>

class StartScene : public Scene {
  public:
    StartScene(Game &game, Client::EventHub &events, SessionManager &session, SceneManager &sceneManager);

    void OnEnter() override;
    void OnExit() override;
    void Update(float dt) override;
    void Render() override;

  private:
    // Input (Update only)
    void UpdateMenuButtons(Vector2 mouse);
    void UpdateInviteToast(Vector2 mouse);

    // Draw only (Render only)
    void RenderBackground(int screenW, int screenH);
    void RenderTitle(int screenW, int screenH);
    void RenderMenuButtons(int screenW, int screenH, Vector2 mouse);
    void RenderInviteToast(int screenW, int screenH, Vector2 mouse);
    void RenderStatusText(int screenW, int screenH);

    // Steam invite callback — fires when the game is already running
    STEAM_CALLBACK(StartScene, OnJoinRequested, GameRichPresenceJoinRequested_t);

    // Helpers
    void ComputeLayout(int screenW, int screenH);
    void SetStatus(const std::string &msg) { m_statusText = msg; }

  private:
    Game &m_game;
    Client::EventHub &m_events;
    SessionManager &m_session;
    SceneManager &m_sceneManager;
    UI::UIManager m_ui;

    enum class State { Idle, WaitingForLobby };
    State m_state = State::Idle;
    std::string m_statusText;

    struct PendingInvite {
        CSteamID fromId;
        CSteamID lobbyId;
        bool active = false;
    };
    PendingInvite m_pendingInvite;

    // Rectangles written by ComputeLayout(), read by both Update and Render
    struct Layout {
        Rectangle hostBtn;
        Rectangle joinBtn;
        Rectangle toastAccept;
        Rectangle toastDecline;
        Rectangle toastPanel;
    };
    Layout m_layout{};
    int m_cachedW = 0;
    int m_cachedH = 0;
};
