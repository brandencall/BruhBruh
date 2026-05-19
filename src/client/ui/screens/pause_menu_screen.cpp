#include "pause_menu_screen.hpp"
#include "confirm_quit_screen.hpp"
#include "option_menu_screen.hpp"
#include "raylib.h"
#include <algorithm>

namespace UI {

// ---------------------------------------------------------------------------
// Helpers
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
    int sw = GetScreenWidth();
    int sh = GetScreenHeight();

    // All sizing derived from screen dimensions so text always fits.
    // Panel is 34% of screen width — wide enough for "QUIT TO DESKTOP" at any resolution.
    m_scaledPanelW = std::max(400, std::min(680, static_cast<int>(sw * 0.34f)));
    // Button height: tall enough for the label plus generous top/bottom padding
    m_scaledBtnH = std::max(54, static_cast<int>(sh * 0.088f));
    m_scaledBtnGap = std::max(10, static_cast<int>(sh * 0.014f));
    // Horizontal padding inside the panel — keeps buttons away from the border
    m_scaledPadX = std::max(24, static_cast<int>(m_scaledPanelW * 0.09f));
    m_scaledPadTop = std::max(24, static_cast<int>(sh * 0.034f));
    m_scaledHeaderH = std::max(56, static_cast<int>(sh * 0.082f));
    m_scaledSepH = std::max(22, static_cast<int>(sh * 0.032f));
    // Label font — slightly larger base so text is easy to read
    m_scaledLabelSz = std::max(18, static_cast<int>(sh * 0.030f));

    int dangerCount = 0;
    for (auto &b : m_buttons)
        if (b.isDanger)
            dangerCount++;

    float contentH = static_cast<float>(m_scaledPadTop) + static_cast<float>(m_scaledHeaderH) + kDivH + 8.f +
                     static_cast<float>(m_buttons.size()) * m_scaledBtnH +
                     static_cast<float>(m_buttons.size() - 1) * m_scaledBtnGap +
                     (dangerCount > 0 ? m_scaledSepH : 0.f) + 20.f;

    m_panelHeight = static_cast<int>(contentH);

    m_panelRect = {static_cast<float>((float)sw / 2 - m_scaledPanelW / 2),
                   static_cast<float>((float)sh / 2 - (float)m_panelHeight / 2), static_cast<float>(m_scaledPanelW),
                   static_cast<float>(m_panelHeight)};
}

bool PauseMenu::BlocksGameInput() const { return true; }
bool PauseMenu::IsDone() const { return m_done; }

void PauseMenu::OnResume() { m_done = true; }

void PauseMenu::OnOptions() { m_ui.Push(std::make_unique<UI::OptionMenu>(m_game)); }

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
void PauseMenu::Update(float dt) {

    m_openAnim = std::min(1.f, m_openAnim + dt * 7.f);

    if (m_flashTimer > 0.f)
        m_flashTimer -= dt;

    LayoutRects();

    int btnCount = static_cast<int>(m_buttons.size());

    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S))
        m_selectedIndex = (m_selectedIndex + 1) % btnCount;
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W))
        m_selectedIndex = (m_selectedIndex - 1 + btnCount) % btnCount;

    Vector2 mouse = GetMousePosition();
    float btnAreaY = m_panelRect.y + m_scaledPadTop + m_scaledHeaderH + kDivH + 8.f;
    float y = btnAreaY;

    for (int i = 0; i < btnCount; i++) {
        if (i > 0 && m_buttons[i].isDanger && !m_buttons[i - 1].isDanger)
            y += static_cast<float>(m_scaledSepH);

        Rectangle btnR = {m_panelRect.x + m_scaledPadX, y, m_panelRect.width - m_scaledPadX * 2.f,
                          static_cast<float>(m_scaledBtnH)};

        if (CheckCollisionPointRec(mouse, btnR))
            m_selectedIndex = i;

        y += m_scaledBtnH + m_scaledBtnGap;
    }

    bool confirm = IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE);
    if (!confirm && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        float cy = btnAreaY;
        for (int i = 0; i < btnCount; i++) {
            if (i > 0 && m_buttons[i].isDanger && !m_buttons[i - 1].isDanger)
                cy += static_cast<float>(m_scaledSepH);
            Rectangle btnR = {m_panelRect.x + m_scaledPadX, cy, m_panelRect.width - m_scaledPadX * 2.f,
                              static_cast<float>(m_scaledBtnH)};
            if (CheckCollisionPointRec(mouse, btnR)) {
                m_selectedIndex = i;
                confirm = true;
                break;
            }
            cy += m_scaledBtnH + m_scaledBtnGap;
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

// ---------------------------------------------------------------------------
// Render
// ---------------------------------------------------------------------------
void PauseMenu::Render() {
    float ease = EaseOutQuart(m_openAnim);

    // Dimmed backdrop matching main menu background colour
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), ColorAlpha({10, 10, 16, 255}, 0.72f * ease));

    // Vignette rings — mirrors RenderBackground() in MainMenuScene
    int sw = GetScreenWidth();
    int sh = GetScreenHeight();
    for (int r = sh; r > 0; r -= sh / 8) {
        unsigned char alpha = static_cast<unsigned char>(60.0f * (1.0f - (float)r / sh) * ease);
        DrawCircle(sw / 2, sh / 2, static_cast<float>(r), {0, 0, 0, alpha});
    }

    DrawPanel();
}

void PauseMenu::DrawPanel() const {
    float ease = EaseOutQuart(m_openAnim);

    // Slide in from top, same feel as main menu buttons fading in
    float offsetY = (1.f - ease) * -30.f;
    Rectangle r = m_panelRect;
    r.y += offsetY;

    // Panel background — same dark base as main menu {10, 10, 16}
    DrawRectangleRec(r, {18, 18, 28, 245});

    // Top accent bar — blue, matching primary button colour
    DrawRectangle(static_cast<int>(r.x), static_cast<int>(r.y), static_cast<int>(r.width), 2, kAccent);

    // Border — same muted blue-grey used for button borders on main menu
    DrawRectangleLinesEx(r, 1.5f, kBorder);

    // Corner brackets
    DrawCorners(r, kAccent, kCornerLen, kCornerThick);

    // Subtle scanline overlay
    for (int py = static_cast<int>(r.y); py < static_cast<int>(r.y + r.height); py += 4)
        DrawRectangle(static_cast<int>(r.x), py, static_cast<int>(r.width), 1, {0, 0, 0, 18});

    DrawHeader();
    DrawButtons();
}

void PauseMenu::DrawHeader() const {
    float ease = EaseOutQuart(m_openAnim);
    float offsetY = (1.f - ease) * -30.f;
    float y = m_panelRect.y + offsetY + m_scaledPadTop;

    // Title — centred, matching main menu title style
    std::string title = TitleText();
    int titleSz = std::max(26, static_cast<int>(GetScreenHeight() * 0.050f));
    int titleW = MeasureText(title.c_str(), titleSz);
    DrawText(title.c_str(), static_cast<int>(m_panelRect.x + m_panelRect.width * 0.5f - titleW * 0.5f),
             static_cast<int>(y + 2.f), titleSz, kTextPrimary);

    // Context tag — small muted label top-right
    std::string tag = ContextTag();
    int tagW = MeasureText(tag.c_str(), 10);
    DrawText(tag.c_str(), static_cast<int>(m_panelRect.x + m_panelRect.width - m_scaledPadX - tagW),
             static_cast<int>(y + 4.f), 10, kTextMuted);

    // Horizontal divider
    float divY = y + m_scaledHeaderH;
    float lineW = m_panelRect.width * 0.60f;
    float lineX = m_panelRect.x + (m_panelRect.width - lineW) * 0.5f;
    DrawRectangle(static_cast<int>(lineX), static_cast<int>(divY), static_cast<int>(lineW), 1, {70, 70, 90, 255});
}

void PauseMenu::DrawButtons() const {
    float ease = EaseOutQuart(m_openAnim);
    float offsetY = (1.f - ease) * -30.f;

    float y = m_panelRect.y + offsetY + m_scaledPadTop + m_scaledHeaderH + kDivH + 8.f;
    int btnCount = static_cast<int>(m_buttons.size());

    for (int i = 0; i < btnCount; i++) {
        const auto &btn = m_buttons[i];

        // Thin rule before first danger button
        if (i > 0 && btn.isDanger && !m_buttons[i - 1].isDanger) {
            DrawRectangle(static_cast<int>(m_panelRect.x + m_scaledPadX),
                          static_cast<int>(y + m_scaledSepH * 0.5f - 0.5f),
                          static_cast<int>(m_panelRect.width - m_scaledPadX * 2.f), 1, {70, 70, 90, 255});
            y += static_cast<float>(m_scaledSepH);
        }

        Rectangle btnR = {m_panelRect.x + m_scaledPadX, y, m_panelRect.width - m_scaledPadX * 2.f,
                          static_cast<float>(m_scaledBtnH)};

        bool selected = (i == m_selectedIndex);
        bool flashing = (i == m_flashIndex && m_flashTimer > 0.f);

        Color fill;
        if (btn.isDanger) {
            fill = (selected || flashing) ? Color{140, 50, 50, 255} : Color{90, 30, 30, 255};
        } else {
            fill = (selected || flashing) ? Color{70, 120, 220, 255} : Color{45, 80, 160, 255};
        }
        if (flashing)
            fill = ColorAlpha(fill, 0.75f);

        DrawRectangleRec(btnR, fill);

        Color borderCol =
            (selected || flashing) ? (btn.isDanger ? Color{200, 80, 80, 255} : Color{120, 160, 255, 255}) : kBorder;
        DrawRectangleLinesEx(btnR, 1.5f, borderCol);

        // Label — always measured and centred so it can never overflow
        int labelSz = m_scaledLabelSz;
        int labelW = MeasureText(btn.label.c_str(), labelSz);
        // If the label is wider than the button interior, shrink font until it fits
        int interiorW = static_cast<int>(btnR.width) - m_scaledPadX * 2;
        while (labelW > interiorW && labelSz > 12) {
            --labelSz;
            labelW = MeasureText(btn.label.c_str(), labelSz);
        }
        DrawText(btn.label.c_str(), static_cast<int>(btnR.x + btnR.width * 0.5f - labelW * 0.5f),
                 static_cast<int>(btnR.y + btnR.height * 0.5f - labelSz * 0.5f), labelSz, WHITE);

        // Hint text (right-aligned)
        std::string hint;
        if (i == 0 && m_cfg.context == MenuContext::InGame)
            hint = "ESC";
        else if (btn.label == "LEAVE" || btn.label == "LEAVE LOBBY")
            hint = LeaveHint();

        if (!hint.empty()) {
            int hw = MeasureText(hint.c_str(), 10);
            DrawText(hint.c_str(), static_cast<int>(btnR.x + btnR.width - hw - 10.f),
                     static_cast<int>(btnR.y + btnR.height * 0.5f - 5.f), 10, kTextMuted);
        }

        y += m_scaledBtnH + m_scaledBtnGap;
    }
}

// ---------------------------------------------------------------------------
// Decorative corner brackets
// ---------------------------------------------------------------------------
void PauseMenu::DrawCorners(Rectangle r, Color c, float len, float thick) const {
    int t = static_cast<int>(thick);
    int L = static_cast<int>(len);
    int x0 = static_cast<int>(r.x) - 1;
    int y0 = static_cast<int>(r.y) - 1;
    int x1 = static_cast<int>(r.x + r.width);
    int y1 = static_cast<int>(r.y + r.height);

    DrawRectangle(x0, y0, L, t, c);
    DrawRectangle(x0, y0, t, L, c);
    DrawRectangle(x1 - L, y0, L, t, c);
    DrawRectangle(x1 - t, y0, t, L, c);
    DrawRectangle(x0, y1 - t, L, t, c);
    DrawRectangle(x0, y1 - L, t, L, c);
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
