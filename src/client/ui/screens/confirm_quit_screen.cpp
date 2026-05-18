#include "confirm_quit_screen.hpp"
#include "../../utils/text_utils.hpp"

namespace UI {

bool ConfirmQuitScreen::BlocksGameInput() const { return true; }
bool ConfirmQuitScreen::IsDone() const { return m_done; }

void ConfirmQuitScreen::Update(float dt) {
    if (IsKeyPressed(KEY_ESCAPE))
        m_done = true;

    Vector2 mouse = GetMousePosition();
    bool clicked = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    ComputeLayout();

    if (clicked) {
        if (CheckCollisionPointRec(mouse, m_yesBtn)) {
            m_onConfirm();
        } else if (CheckCollisionPointRec(mouse, m_noBtn)) {
            m_done = true;
        }
    }
}

void ConfirmQuitScreen::Render() {
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();
    ComputeLayout();
    Vector2 mouse = GetMousePosition();

    // Dimmed background
    DrawRectangle(0, 0, screenW, screenH, {0, 0, 0, 180});

    // Panel
    DrawRectangleRec(m_panel, {25, 25, 35, 255});
    DrawRectangleLinesEx(m_panel, 1.5f, {70, 70, 100, 255});

    // Message
    utils::DrawTextCentered("Are you sure you want to quit?", m_panel.x + m_panel.width * 0.5f,
                            m_panel.y + m_panel.height * 0.35f, 20, WHITE);

    // Yes button
    bool yesHov = CheckCollisionPointRec(mouse, m_yesBtn);
    DrawRectangleRec(m_yesBtn, yesHov ? Color{180, 50, 50, 255} : Color{120, 30, 30, 255});
    DrawRectangleLinesEx(m_yesBtn, 1, {220, 80, 80, 255});
    utils::DrawTextCentered("Yes", m_yesBtn.x + m_yesBtn.width * 0.5f, m_yesBtn.y + m_yesBtn.height * 0.5f - 8, 18,
                            WHITE);

    // No button
    bool noHov = CheckCollisionPointRec(mouse, m_noBtn);
    DrawRectangleRec(m_noBtn, noHov ? Color{55, 55, 75, 255} : Color{35, 35, 50, 255});
    DrawRectangleLinesEx(m_noBtn, 1, {80, 80, 110, 255});
    utils::DrawTextCentered("No", m_noBtn.x + m_noBtn.width * 0.5f, m_noBtn.y + m_noBtn.height * 0.5f - 8, 18, WHITE);
}

void ConfirmQuitScreen::ComputeLayout() {
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();

    float panelW = screenW * 0.28f;
    float panelH = screenH * 0.20f;
    m_panel = {screenW * 0.5f - panelW * 0.5f, screenH * 0.5f - panelH * 0.5f, panelW, panelH};

    float btnW = panelW * 0.28f;
    float btnH = panelH * 0.28f;
    float btnY = m_panel.y + panelH * 0.62f;
    float cx = m_panel.x + panelW * 0.5f;

    m_yesBtn = {cx - btnW - screenW * 0.012f, btnY, btnW, btnH};
    m_noBtn = {cx + screenW * 0.012f, btnY, btnW, btnH};
}

} // namespace UI
