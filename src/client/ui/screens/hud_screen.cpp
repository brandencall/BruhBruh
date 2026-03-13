#include "hud_screen.hpp"
#include <string>

namespace UI {

HudScreen::HudScreen(const state::PlayerState &localPlayer) : m_localPlayer(localPlayer) {}

void HudScreen::Render() {
    int h = GetScreenHeight();

    std::string health = "HP: " + std::to_string(static_cast<int>(m_localPlayer.health));
    int padding = 10;
    int fontSize = 24;

    DrawText(health.c_str(), padding, h - fontSize - padding, fontSize, RAYWHITE);
}

} // namespace UI
