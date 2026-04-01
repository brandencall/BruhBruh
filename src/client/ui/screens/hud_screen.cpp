#include "hud_screen.hpp"
#include <string>

namespace UI {

HudScreen::HudScreen(const state::PlayerState &localPlayer) : m_localPlayer(localPlayer) {}

void HudScreen::Render() {
    int h = GetScreenHeight();
    std::string health = "HP: " + std::to_string(static_cast<int>(m_localPlayer.health));
    std::string currentWalls = "Walls: " + std::to_string(static_cast<int>(m_localPlayer.currentAvaliableWalls));
    int padding = 10;
    int fontSize = 24;
    int gap = 20;

    DrawText(health.c_str(), padding, h - fontSize - padding, fontSize, RAYWHITE);
    DrawText(currentWalls.c_str(), padding + MeasureText(health.c_str(), fontSize) + gap, h - fontSize - padding,
             fontSize, RAYWHITE);
}

} // namespace UI
