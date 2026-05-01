#include "death_screen.hpp"
#include <string>

namespace UI {

DeathScreen::DeathScreen(const state::PlayerState &localPlayer) : m_localPlayer(localPlayer) {}

void DeathScreen::Render() {
    int w = GetScreenWidth(), h = GetScreenHeight();

    DrawRectangle(0, 0, w, h, {0, 0, 0, 160});

    const char *title = "YOU DIED";
    int titleSize = 64;
    DrawText(title, (w - MeasureText(title, titleSize)) / 2, h / 2 - 60, titleSize, RED);

    std::string timer = "Respawning in " + std::to_string(static_cast<int>(m_localPlayer.respawnTimer) + 1) + "...";
    DrawText(timer.c_str(), (w - MeasureText(timer.c_str(), 24)) / 2, h / 2 + 20, 24, RAYWHITE);
}

} // namespace UI
