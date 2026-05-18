#include "pause_menu_screen.hpp"
#include "confirm_quit_screen.hpp"
#include "raylib.h"
#include <algorithm>
#include <iostream>

namespace UI {

// ---------------------------------------------------------------------------
// Helpers — not in header
// ---------------------------------------------------------------------------

static float EaseOutQuart(float t) {
    t = 1.f - t;
    return 1.f - t * t * t * t;
}

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------
PauseMenu::PauseMenu(Game &game, UIManager &ui, const PauseMenuConfig &cfg) : m_game(game), m_ui(ui), m_cfg(cfg) {
    BuildButtons();
    LayoutRects();
}

void PauseMenu::BuildButtons() {
    m_buttons.clear();

    if (m_cfg.context == MenuContext::InGame)
        m_buttons.push_back({"RESUME", [this]() { OnResume(); }, false});

    m_buttons.push_back({"OPTIONS", [this]() { OnOptions(); }, false});

    if (m_cfg.context != MenuContext::MainMenu)
        m_buttons.push_back({"LEAVE", [this]() { OnLeave(); }, true});

    m_buttons.push_back({"QUIT TO DESKTOP", [this]() { OnQuitDesktop(); }, true});
}

void PauseMenu::LayoutRects() {
    // Count how many buttons are danger (need a separator gap before them)
    int dangerCount = 0;
    for (auto &b : m_buttons)
        if (b.isDanger)
            dangerCount++;

    float contentH = kPadTop + kHeaderH + kDivH + 8.f // divider + small gap
                     + static_cast<float>(m_buttons.size()) * kBtnH +
                     static_cast<float>(m_buttons.size() - 1) * kBtnGap +
                     (dangerCount > 0 ? kSepH : 0.f) // separator before danger btn
                     + 16.f;                         // bottom pad before footer

    m_panelHeight = static_cast<int>(contentH);

    int sw = GetScreenWidth();
    int sh = GetScreenHeight();
    m_panelRect = {static_cast<float>((float)sw / 2 - (float)m_panelWidth / 2),
                   static_cast<float>((float)sh / 2 - (float)m_panelHeight / 2), static_cast<float>(m_panelWidth),
                   static_cast<float>(m_panelHeight)};
}

bool PauseMenu::BlocksGameInput() const { return true; }
bool PauseMenu::IsDone() const { return m_done; }

void PauseMenu::OnResume() { m_done = true; }

void PauseMenu::OnOptions() { std::cout << "The Options button was pressed" << std::endl; }

void PauseMenu::OnLeave() {
    m_ui.Push(std::make_unique<UI::ConfirmQuitScreen>([this]() {
        if (m_cfg.onLeave)
            m_cfg.onLeave();
        m_game.GetSessionManager()->ReturnToMainMenu();
    }));
}

void PauseMenu::OnQuitDesktop() {
    m_ui.Push(std::make_unique<UI::ConfirmQuitScreen>([this]() {
        if (m_cfg.onQuitDesktop)
            m_cfg.onQuitDesktop();
        m_game.RequestQuit();
    }));
}

// ---------------------------------------------------------------------------
// Update
// ---------------------------------------------------------------------------
void PauseMenu::Update(float _) {
    float dt = GetFrameTime();

    // Animate open
    m_openAnim = std::min(1.f, m_openAnim + dt * 7.f);

    // Flash timer
    if (m_flashTimer > 0.f)
        m_flashTimer -= dt;

    // Recalculate layout in case window was resized
    LayoutRects();

    int btnCount = static_cast<int>(m_buttons.size());

    // --- Keyboard navigation ---
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S))
        m_selectedIndex = (m_selectedIndex + 1) % btnCount;
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W))
        m_selectedIndex = (m_selectedIndex - 1 + btnCount) % btnCount;

    // --- Mouse hover ---
    Vector2 mouse = GetMousePosition();
    float y = m_panelRect.y + kPadTop + kHeaderH + kDivH + 8.f;

    for (int i = 0; i < btnCount; i++) {
        // Insert separator gap before first danger button
        if (i > 0 && m_buttons[i].isDanger && !m_buttons[i - 1].isDanger)
            y += kSepH;

        Rectangle btnR = {m_panelRect.x + kPadX, y, m_panelRect.width - kPadX * 2.f, kBtnH};

        if (CheckCollisionPointRec(mouse, btnR))
            m_selectedIndex = i;

        y += kBtnH + kBtnGap;
    }

    // --- Confirm (mouse click or Enter/Space) ---
    bool confirm = IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE);
    if (!confirm && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        // Re-derive hovered button rect to confirm click
        float cy = m_panelRect.y + kPadTop + kHeaderH + kDivH + 8.f;
        for (int i = 0; i < btnCount; i++) {
            if (i > 0 && m_buttons[i].isDanger && !m_buttons[i - 1].isDanger)
                cy += kSepH;
            Rectangle btnR = {m_panelRect.x + kPadX, cy, m_panelRect.width - kPadX * 2.f, kBtnH};
            if (CheckCollisionPointRec(mouse, btnR)) {
                m_selectedIndex = i;
                confirm = true;
                break;
            }
            cy += kBtnH + kBtnGap;
        }
    }

    if (confirm && m_selectedIndex >= 0 && m_selectedIndex < btnCount) {
        m_flashIndex = m_selectedIndex;
        m_flashTimer = 0.12f;
        if (m_buttons[m_selectedIndex].callback)
            m_buttons[m_selectedIndex].callback();
    }

    if (IsKeyPressed(KEY_ESCAPE)) {
        m_done = true;
    }
}

void PauseMenu::Render() {
    float ease = EaseOutQuart(m_openAnim);

    // -- Dimmed backdrop --
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), ColorAlpha({0, 0, 0, 255}, 0.55f * ease));

    // -- Grid lines on backdrop --
    int sw = GetScreenWidth(), sh = GetScreenHeight();
    int gridStep = 32;
    Color gridCol = {255, 255, 255, static_cast<unsigned char>(6 * ease)};
    for (int x = 0; x < sw; x += gridStep)
        DrawLine(x, 0, x, sh, gridCol);
    for (int y = 0; y < sh; y += gridStep)
        DrawLine(0, y, sw, y, gridCol);

    DrawPanel();
}

void PauseMenu::DrawPanel() const {
    float ease = EaseOutQuart(m_openAnim);

    // Slide in from top
    float offsetY = (1.f - ease) * -30.f;
    Rectangle r = m_panelRect;
    r.y += offsetY;

    // Background
    DrawRectangleRec(r, kBgPanel);

    // Top accent bar
    DrawRectangle(static_cast<int>(r.x), static_cast<int>(r.y), static_cast<int>(r.width), 2, kAccent);

    // Border
    DrawRectangleLinesEx(r, 1.f, kBorder);

    // Corner brackets
    DrawCorners(r, kAccent, kCornerLen, kCornerThick);

    // Scanline overlay (every other pixel row, very subtle)
    for (int py = static_cast<int>(r.y); py < static_cast<int>(r.y + r.height); py += 4)
        DrawRectangle(static_cast<int>(r.x), py, static_cast<int>(r.width), 1, {0, 0, 0, 18});

    DrawHeader();
    DrawButtons();
}

void PauseMenu::DrawHeader() const {
    float ease = EaseOutQuart(m_openAnim);
    float offsetY = (1.f - ease) * -30.f;
    float y = m_panelRect.y + offsetY + kPadTop;

    // Orange dot
    float dotX = m_panelRect.x + kPadX;
    float dotY = y + 8.f;
    DrawRectangle(static_cast<int>(dotX), static_cast<int>(dotY), 8, 8, kAccent);

    // Title
    DrawText(TitleText().c_str(), static_cast<int>(dotX + 18.f), static_cast<int>(y + 2.f), 22, kTextPrimary);

    // Context tag (top-right)
    std::string tag = ContextTag();
    int tagW = MeasureText(tag.c_str(), 10);
    DrawText(tag.c_str(), static_cast<int>(m_panelRect.x + m_panelRect.width - kPadX - tagW),
             static_cast<int>(y + 10.f), 10, kAccent);

    // Horizontal divider
    float divY = y + kHeaderH;
    DrawRectangle(static_cast<int>(m_panelRect.x + kPadX), static_cast<int>(divY),
                  static_cast<int>(m_panelRect.width - kPadX * 2.f), 1, kBorder);
}

void PauseMenu::DrawButtons() const {
    float ease = EaseOutQuart(m_openAnim);
    float offsetY = (1.f - ease) * -30.f;

    float y = m_panelRect.y + offsetY + kPadTop + kHeaderH + kDivH + 8.f;
    int btnCount = static_cast<int>(m_buttons.size());

    for (int i = 0; i < btnCount; i++) {
        const auto &btn = m_buttons[i];

        // Separator gap before first danger button
        if (i > 0 && btn.isDanger && !m_buttons[i - 1].isDanger) {
            // Draw a thin rule
            DrawRectangle(static_cast<int>(m_panelRect.x + kPadX), static_cast<int>(y + kSepH * 0.5f - 0.5f),
                          static_cast<int>(m_panelRect.width - kPadX * 2.f), 1, kBorder);
            y += kSepH;
        }

        Rectangle btnR = {m_panelRect.x + kPadX, y, m_panelRect.width - kPadX * 2.f, kBtnH};

        bool selected = (i == m_selectedIndex);
        bool flashing = (i == m_flashIndex && m_flashTimer > 0.f);

        Color accentCol = btn.isDanger ? kDanger : kAccent;

        // Hover/selected fill
        if (selected || flashing) {
            Color fillCol = ColorAlpha(accentCol, flashing ? 0.22f : 0.10f);
            DrawRectangleRec(btnR, fillCol);
        }

        // Left highlight bar on selected
        if (selected)
            DrawRectangle(static_cast<int>(btnR.x), static_cast<int>(btnR.y), 3, static_cast<int>(kBtnH), accentCol);

        // Border
        Color borderCol = selected ? ColorAlpha(accentCol, 0.55f) : kBorder;
        DrawRectangleLinesEx(btnR, 0.5f, borderCol);

        // Label
        Color labelCol = btn.isDanger ? ColorLerp({180, 60, 60, 255}, kDanger, selected ? 1.f : 0.6f)
                                      : (selected ? kTextPrimary : ColorAlpha(kTextPrimary, 0.7f));

        DrawText(btn.label.c_str(), static_cast<int>(btnR.x + 14.f), static_cast<int>(btnR.y + kBtnH * 0.5f - 8.f), 15,
                 labelCol);

        // Hint text (right-aligned, context-dependent)
        std::string hint;
        if (i == 0 && m_cfg.context == MenuContext::InGame)
            hint = "ESC";
        else if (btn.label == "LEAVE" || btn.label == "LEAVE LOBBY")
            hint = LeaveHint();

        if (!hint.empty()) {
            int hw = MeasureText(hint.c_str(), 10);
            DrawText(hint.c_str(), static_cast<int>(btnR.x + btnR.width - hw - 10.f),
                     static_cast<int>(btnR.y + kBtnH * 0.5f - 5.f), 10, kTextMuted);
        }

        y += kBtnH + kBtnGap;
    }
}

// ---------------------------------------------------------------------------
// Decorative corner brackets
// ---------------------------------------------------------------------------
void PauseMenu::DrawCorners(Rectangle r, Color c, float len, float thick) const {
    int t = static_cast<int>(thick);
    int L = static_cast<int>(len);
    int x0 = static_cast<int>(r.x) - 1;
    int y0 = static_cast<int>(r.y) - 1; // overlap the accent bar
    int x1 = static_cast<int>(r.x + r.width);
    int y1 = static_cast<int>(r.y + r.height);

    // Top-left
    DrawRectangle(x0, y0, L, t, c);
    DrawRectangle(x0, y0, t, L, c);
    // Top-right
    DrawRectangle(x1 - L, y0, L, t, c);
    DrawRectangle(x1 - t, y0, t, L, c);
    // Bottom-left
    DrawRectangle(x0, y1 - t, L, t, c);
    DrawRectangle(x0, y1 - L, t, L, c);
    // Bottom-right
    DrawRectangle(x1 - L, y1 - t, L, t, c);
    DrawRectangle(x1 - t, y1 - L, t, L, c);
}

// ---------------------------------------------------------------------------
// String helpers
// ---------------------------------------------------------------------------
std::string PauseMenu::ContextTag() const {
    switch (m_cfg.context) {
    case MenuContext::InGame:
        return "IN-GAME";
    case MenuContext::Lobby:
        return "WAITING";
    case MenuContext::MainMenu:
        return "MAIN MENU";
    }
    return "";
}

std::string PauseMenu::TitleText() const {
    switch (m_cfg.context) {
    case MenuContext::InGame:
        return "PAUSED";
    case MenuContext::Lobby:
        return "LOBBY";
    case MenuContext::MainMenu:
        return "MENU";
    }
    return "";
}

std::string PauseMenu::LeaveHint() const {
    switch (m_cfg.context) {
    case MenuContext::InGame:
        return "-> LOBBY";
    case MenuContext::Lobby:
        return "-> MENU";
    case MenuContext::MainMenu:
        return "";
    }
    return "";
}

} // namespace UI
