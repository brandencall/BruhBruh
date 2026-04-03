#include "hud_screen.hpp"
#include "../../utils/text_utils.hpp"
#include "raylib.h"
#include <string>

namespace UI {

HudScreen::HudScreen(const state::PlayerState &localPlayer, const std::vector<System::Feed> &feed,
                     const float &gameTime)
    : m_localPlayer(localPlayer), m_feed(feed), m_gameTime(gameTime) {}

void HudScreen::Render() {
    int screenH = GetScreenHeight();
    int screenW = GetScreenWidth();
    int padding = 10;
    int fontSize = 24;
    int gap = 20;

    // ── Bottom-left HUD ───────────────────────────────────────────────────────
    std::string health = "HP: " + std::to_string(static_cast<int>(m_localPlayer.health));
    std::string currentWalls = "Walls: " + std::to_string(static_cast<int>(m_localPlayer.currentAvaliableWalls));

    DrawText(health.c_str(), padding, screenH - fontSize - padding, fontSize, RAYWHITE);

    DrawText(currentWalls.c_str(), padding + MeasureText(health.c_str(), fontSize) + gap, screenH - fontSize - padding,
             fontSize, RAYWHITE);

    // ── Kill feed (above HUD, left side) ─────────────────────────────────────
    int feedFontSize = 18;
    int feedLineH = feedFontSize + 6; // line height with a little breathing room
    int feedBaseY = screenH - fontSize - padding * 2 - feedLineH;

    for (int i = (int)m_feed.size() - 1; i >= 0; i--) {
        const auto &entry = m_feed[i];
        std::string line = entry.killer + " > " + entry.victim;

        // Fade older entries: newest = full white, oldest = ~40% opacity
        float t = (m_feed.size() > 1) ? (float)(m_feed.size() - 1 - i) / (float)(m_feed.size() - 1) : 0.0f;
        unsigned char alpha = (unsigned char)(255 - t * 150);
        Color color = {255, 255, 255, alpha};

        DrawText(line.c_str(), padding, feedBaseY - ((int)m_feed.size() - 1 - i) * feedLineH, feedFontSize, color);
    }
    int lobbyMins = (int)(m_gameTime / 60);
    int lobbySeconds = (int)m_gameTime % 60;
    std::string lobbyTime = "Time: " + std::to_string(lobbyMins) + ":" + std::to_string(lobbySeconds);
    utils::DrawTextCentered(lobbyTime.c_str(), screenW * 0.5, 1 + padding, fontSize, RAYWHITE);
}

} // namespace UI
