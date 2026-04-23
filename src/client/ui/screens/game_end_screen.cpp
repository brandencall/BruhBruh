#include "game_end_screen.hpp"
#include <raylib.h>

namespace UI {

GameEndScreen::GameEndScreen(const GameEndData &data)
    : m_countdown(data.countdown), m_playerCount(data.playerCount), m_rankings(data.rankings) {}

void GameEndScreen::Render() {
    RenderBackground();
    RenderTitle();
    RenderRankings();
    RenderCountdown();
}

bool GameEndScreen::BlocksGameInput() const { return true; }

bool GameEndScreen::IsDone() const { return m_countdown <= 0.0f; }

void GameEndScreen::UpdateCountdown(float serverCountdown) { m_countdown = serverCountdown; }

void GameEndScreen::RenderBackground() const {
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), {0, 0, 0, 180});
}

void GameEndScreen::RenderTitle() const {
    const char *title = "GAME OVER";
    int fontSize = 48;
    int textWidth = MeasureText(title, fontSize);
    int x = (GetScreenWidth() - textWidth) / 2;
    DrawText(title, x, 80, fontSize, RAYWHITE);
}

void GameEndScreen::RenderRankings() const {
    const int startY = 200;
    const int rowHeight = 50;
    const int fontSize = 28;

    // Column headers
    DrawText("RANK", 160, startY - 35, 20, GRAY);
    DrawText("NAME", 280, startY - 35, 20, GRAY);
    DrawText("KILLS", 520, startY - 35, 20, GRAY);
    DrawText("DEATHS", 620, startY - 35, 20, GRAY);

    for (int i = 0; i < m_playerCount; i++) {
        const auto &r = m_rankings[i];
        Color rowColor = (i == 0) ? GOLD : RAYWHITE;
        int y = startY + i * rowHeight;

        DrawText(TextFormat("#%d", i + 1), 160, y, fontSize, rowColor);
        DrawText(r.name, 280, y, fontSize, rowColor);
        DrawText(TextFormat("%d", r.score.kills), 530, y, fontSize, rowColor);
        DrawText(TextFormat("%d", r.score.deaths), 640, y, fontSize, rowColor);
    }
}

void GameEndScreen::RenderCountdown() const {
    float remaining = m_countdown;
    if (remaining < 0.0f)
        remaining = 0.0f;

    const char *text = TextFormat("Returning to lobby in %.0fs", remaining);
    int textWidth = MeasureText(text, 22);
    int x = (GetScreenWidth() - textWidth) / 2;
    DrawText(text, x, GetScreenHeight() - 80, 22, GRAY);
}

} // namespace UI
