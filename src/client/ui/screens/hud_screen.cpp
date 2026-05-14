#include "hud_screen.hpp"
#include "raylib.h"
#include <string>

namespace UI {

HudScreen::HudScreen(const state::PlayerState &localPlayer, const float &gameTime, Client::EventHub &events)
    : m_localPlayer(localPlayer), m_gameTime(gameTime), m_events(events) {
    m_deathSub = events.playerDied.Subscribe(
        [this](const client::PlayerDiedEvent &e) { m_killFeed.Push(e.data.killer.name, e.data.victim.name); });
}

void HudScreen::Update(float dt) { m_killFeed.Update(dt); }

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

    const std::vector<System::Feed> feed = m_killFeed.GetFeed();

    for (int i = (int)feed.size() - 1; i >= 0; i--) {
        const auto &entry = feed[i];
        std::string line = entry.killer + " > " + entry.victim;

        // Fade older entries: newest = full white, oldest = ~40% opacity
        float t = (feed.size() > 1) ? (float)(feed.size() - 1 - i) / (float)(feed.size() - 1) : 0.0f;
        unsigned char alpha = (unsigned char)(255 - t * 150);
        Color color = {255, 255, 255, alpha};

        DrawText(line.c_str(), padding, feedBaseY - ((int)feed.size() - 1 - i) * feedLineH, feedFontSize, color);
    }
    RenderGameTime(screenW);
}

void HudScreen::RenderGameTime(int screenW) {
    if (m_gameTime < 0)
        return;

    int fontSize = 30;
    int padding = 14;
    int lobbyMins = (int)(m_gameTime / 60);
    int lobbySeconds = (int)m_gameTime % 60;

    char buffer[32];
    snprintf(buffer, sizeof(buffer), "Time: %d:%02d", lobbyMins, lobbySeconds);

    int textWidth = MeasureText(buffer, fontSize);
    int textX = (screenW - textWidth) / 2;
    int textY = 1 + padding;

    int rectPaddingX = 12;
    int rectPaddingY = 6;

    Rectangle rect = {(float)(textX - rectPaddingX), (float)(textY - rectPaddingY),
                      (float)(textWidth + rectPaddingX * 2), (float)(fontSize + rectPaddingY * 2)};

    DrawRectangleRounded(rect, 0.3f, 8, Fade(BLACK, 0.5f));

    DrawText(buffer, textX, textY, fontSize, RAYWHITE);
}

} // namespace UI
