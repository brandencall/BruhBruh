#include "hud_screen.hpp"
#include <string>

namespace UI {

HudScreen::HudScreen(const state::PlayerState &localPlayer, const std::vector<System::Feed> &feed)
    : m_localPlayer(localPlayer), m_feed(feed) {}

void HudScreen::Render() {
    int screenH = GetScreenHeight();
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
}

} // namespace UI
