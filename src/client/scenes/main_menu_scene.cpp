#include "main_menu_scene.hpp"
#include "../ui/screens/join_screen.hpp"
#include "../ui/screens/pause_menu_screen.hpp"
#include "../utils/text_utils.hpp"
#include "raylib.h"
#include <cassert>
#include <iostream>

MainMenuScene::MainMenuScene(Game &game) : m_game(game) {}

void MainMenuScene::OnEnter() {
    m_state = State::Idle;
    m_statusText.clear();
    m_pendingInvite.active = false;
    m_ui.Clear();
    m_game.GetSessionManager()->GetLobby()->SetCallbacks({
        .onInviteAccepted = [this](CSteamID from, CSteamID lobbyId) { m_pendingInvite = {from, lobbyId, true}; },
        .onJoinRequested =
            [this](CSteamID lobbyId) {
                std::cout << "Invite recieved" << std::endl;
                m_pendingInvite = {CSteamID(), lobbyId, true};
            },
    });
}

void MainMenuScene::OnExit() {
    Scene::OnExit();
    m_pendingInvite.active = false;
    m_game.GetSessionManager()->GetLobby()->SetCallbacks({});
    m_ui.Clear();
}

// ─────────────────────────────────────────────────────────────────────────────
// Layout — recomputed only when window size changes
// ─────────────────────────────────────────────────────────────────────────────

void MainMenuScene::ComputeLayout(int screenW, int screenH) {
    if (screenW == m_cachedW && screenH == m_cachedH)
        return;
    m_cachedW = screenW;
    m_cachedH = screenH;

    float btnW = screenW * 0.22f;
    float btnH = screenH * 0.072f;
    float cx = screenW * 0.5f;

    m_layout.hostBtn = {cx - btnW * 0.5f, screenH * 0.50f, btnW, btnH};
    m_layout.joinBtn = {cx - btnW * 0.5f, screenH * 0.50f + btnH + screenH * 0.025f, btnW, btnH};

    // Toast panel — bottom centre
    float toastW = screenW * 0.40f;
    float toastH = screenH * 0.13f;
    float toastX = cx - toastW * 0.5f;
    float toastY = screenH * 0.82f;
    m_layout.toastPanel = {toastX, toastY, toastW, toastH};

    float tbtnW = toastW * 0.28f;
    float tbtnH = toastH * 0.38f;
    float tbtnY = toastY + toastH * 0.55f;
    m_layout.toastAccept = {cx - tbtnW - screenW * 0.012f, tbtnY, tbtnW, tbtnH};
    m_layout.toastDecline = {cx + screenW * 0.012f, tbtnY, tbtnW, tbtnH};
}

// ─────────────────────────────────────────────────────────────────────────────
// Steam callback — game already running when invite arrives
// ─────────────────────────────────────────────────────────────────────────────

void MainMenuScene::OnJoinRequested(GameRichPresenceJoinRequested_t *pCallback) {
    // pCallback->m_rgchConnect holds the lobby ID string set via SetRichPresence
    std::cout << "Got join request" << std::endl;
    uint64 lobbyRaw = std::stoull(pCallback->m_rgchConnect);
    m_pendingInvite = {pCallback->m_steamIDFriend, CSteamID(lobbyRaw), true};
}

// ─────────────────────────────────────────────────────────────────────────────
// Update — all input here
// ─────────────────────────────────────────────────────────────────────────────

void MainMenuScene::Update(float dt) {
    m_ui.Update(dt);
    if (m_ui.BlocksGameInput())
        return;

    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();
    ComputeLayout(screenW, screenH);

    Vector2 mouse = GetMousePosition();

    if (m_state == State::Idle)
        UpdateMenuButtons(mouse);

    if (m_pendingInvite.active)
        UpdateInviteToast(mouse);

    if (IsKeyPressed(KEY_ESCAPE) && !m_ui.BlocksGameInput()) {
        UI::PauseMenuConfig cfg;
        cfg.context = UI::MenuContext::MainMenu;
        m_ui.Push(std::make_unique<UI::PauseMenu>(m_game, m_ui, cfg));
    }
}

void MainMenuScene::UpdateMenuButtons(Vector2 mouse) {
    if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        return;

    if (CheckCollisionPointRec(mouse, m_layout.hostBtn)) {
        SetStatus("Creating lobby...");
        m_state = State::WaitingForLobby;

        m_game.GetSessionManager()->HostGame(
            [this]() {
                // onLobbyCreated fires on the Steam callback thread — push scene safely
                assert(m_session.GetTransport());
                assert(m_session.GetHandler());
                m_game.GetSessionManager()->CreateLobby();
            },
            [this](const char *err) {
                SetStatus(std::string("Error: ") + err);
                m_state = State::Idle;
            });
        return;
    }

    if (CheckCollisionPointRec(mouse, m_layout.joinBtn)) {
        m_ui.Push(std::make_unique<UI::JoinScreen>([this](CSteamID lobbyId) {
            SetStatus("Joining lobby...");
            m_state = State::WaitingForLobby;
            m_game.GetSessionManager()->JoinLobby(
                lobbyId,
                [this]() {
                    std::cout << "Pushing the lobby scene when the join button was pushed" << std::endl;
                    // JoinScreen will be cleaned up when MainMenuScene exits
                    assert(m_session.GetTransport());
                    assert(m_session.GetHandler());
                    m_game.GetSessionManager()->CreateLobby();
                },
                [this](const char *err) {
                    SetStatus(std::string("Error: ") + err);
                    m_state = State::Idle;
                });
        }));
    }
}

void MainMenuScene::UpdateInviteToast(Vector2 mouse) {
    if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        return;

    if (CheckCollisionPointRec(mouse, m_layout.toastAccept)) {
        SetStatus("Joining lobby...");
        m_state = State::WaitingForLobby;
        CSteamID lobbyId = m_pendingInvite.lobbyId;
        m_pendingInvite.active = false;

        m_game.GetSessionManager()->JoinLobby(
            lobbyId,
            [this]() {
                std::cout << SteamFriends()->GetPersonaName() << ": Join lobby called.. pushing lobby scene"
                          << std::endl;
                assert(m_session.GetTransport());
                assert(m_session.GetHandler());
                m_game.GetSessionManager()->CreateLobby();
            },
            [this](const char *err) {
                SetStatus(std::string("Error: ") + err);
                m_state = State::Idle;
            });
        return;
    }

    if (CheckCollisionPointRec(mouse, m_layout.toastDecline)) {
        m_pendingInvite.active = false;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Render — draw only, no input queries
// ─────────────────────────────────────────────────────────────────────────────

void MainMenuScene::Render() {
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();
    ComputeLayout(screenW, screenH);
    Vector2 mouse = GetMousePosition();

    BeginDrawing();
    ClearBackground({10, 10, 16, 255});

    RenderBackground(screenW, screenH);
    RenderTitle(screenW, screenH);

    if (m_state == State::Idle)
        RenderMenuButtons(screenW, screenH, mouse);

    RenderStatusText(screenW, screenH);

    if (m_pendingInvite.active)
        RenderInviteToast(screenW, screenH, mouse);

    m_ui.Render();
    EndDrawing();
}

void MainMenuScene::RenderBackground(int screenW, int screenH) {
    // Subtle vignette — concentric darkening rings from centre
    for (int r = screenH; r > 0; r -= screenH / 8) {
        unsigned char alpha = (unsigned char)(80.0f * (1.0f - (float)r / screenH));
        DrawCircle(screenW / 2, screenH / 2, (float)r, {0, 0, 0, alpha});
    }
}

void MainMenuScene::RenderTitle(int screenW, int screenH) {
    // Game title
    const char *title = "BRUHBRUH";
    int titleSz = screenH * 0.10f;
    utils::DrawTextCentered(title, screenW * 0.5f, screenH * 0.22f, titleSz, WHITE);

    // Thin divider line below title
    int lineY = screenH * 0.36f;
    int lineW = screenW * 0.18f;
    int cx = screenW / 2;
    DrawLine(cx - lineW / 2, lineY, cx + lineW / 2, lineY, {70, 70, 90, 255});

    // Player name from Steam
    const char *steamName = SteamFriends()->GetPersonaName();
    int nameSz = screenH * 0.022f;
    utils::DrawTextCentered(steamName, screenW * 0.5f, screenH * 0.385f, nameSz, {160, 160, 180, 255});
}

void MainMenuScene::RenderMenuButtons(int screenW, int screenH, Vector2 mouse) {
    auto drawBtn = [&](Rectangle r, const char *label, bool primary) {
        bool hovered = CheckCollisionPointRec(mouse, r) && !m_ui.BlocksGameInput();

        Color fill = primary ? (hovered ? Color{70, 120, 220, 255} : Color{45, 80, 160, 255})
                             : (hovered ? Color{55, 55, 75, 255} : Color{30, 30, 45, 255});
        Color border = hovered ? Color{120, 160, 255, 255} : Color{70, 70, 100, 255};

        DrawRectangleRec(r, fill);
        DrawRectangleLinesEx(r, 1.5f, border);
        utils::DrawTextCentered(label, r.x + r.width * 0.5f, r.y + r.height * 0.5f - screenH * 0.013f, screenH * 0.026f,
                                WHITE);
    };

    drawBtn(m_layout.hostBtn, "Host Game", true);
    drawBtn(m_layout.joinBtn, "Join Game", false);

    // Quit hint bottom-right
    DrawText("ESC  Quit", screenW - MeasureText("ESC  Quit", 14) - 16, screenH - 28, 14, {60, 60, 80, 255});
}

void MainMenuScene::RenderStatusText(int screenW, int screenH) {
    if (m_statusText.empty())
        return;

    // Animated dots when waiting
    std::string display = m_statusText;
    if (m_state == State::WaitingForLobby) {
        int dots = (int)(GetTime() * 2.0) % 4;
        display += std::string(dots, '.');
    }

    utils::DrawTextCentered(display.c_str(), screenW * 0.5f, screenH * 0.72f, screenH * 0.022f, {160, 160, 180, 255});
}

void MainMenuScene::RenderInviteToast(int screenW, int screenH, Vector2 mouse) {
    const auto &p = m_layout.toastPanel;

    // Panel
    DrawRectangleRec(p, {22, 22, 32, 245});
    DrawRectangleLinesEx(p, 1.5f, {80, 80, 120, 255});

    // "Invited by <name>" text
    const char *fromName = SteamFriends()->GetFriendPersonaName(m_pendingInvite.fromId);
    std::string msg = std::string(fromName) + " invited you to play";
    int msgSz = screenH * 0.021f;
    utils::DrawTextCentered(msg.c_str(), p.x + p.width * 0.5f, p.y + p.height * 0.18f, msgSz, WHITE);

    // Accept button
    bool acceptHovered = CheckCollisionPointRec(mouse, m_layout.toastAccept) && !m_ui.BlocksGameInput();
    DrawRectangleRec(m_layout.toastAccept, acceptHovered ? Color{60, 160, 80, 255} : Color{35, 100, 50, 255});
    DrawRectangleLinesEx(m_layout.toastAccept, 1, {80, 200, 100, 255});
    utils::DrawTextCentered("Accept", m_layout.toastAccept.x + m_layout.toastAccept.width * 0.5f,
                            m_layout.toastAccept.y + m_layout.toastAccept.height * 0.5f - screenH * 0.01f,
                            screenH * 0.020f, WHITE);

    // Decline button
    bool declineHovered = CheckCollisionPointRec(mouse, m_layout.toastDecline);
    DrawRectangleRec(m_layout.toastDecline, declineHovered ? Color{160, 55, 55, 255} : Color{100, 35, 35, 255});
    DrawRectangleLinesEx(m_layout.toastDecline, 1, {200, 80, 80, 255});
    utils::DrawTextCentered("Decline", m_layout.toastDecline.x + m_layout.toastDecline.width * 0.5f,
                            m_layout.toastDecline.y + m_layout.toastDecline.height * 0.5f - screenH * 0.01f,
                            screenH * 0.020f, WHITE);
}
