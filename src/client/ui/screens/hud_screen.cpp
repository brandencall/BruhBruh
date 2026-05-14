#include "hud_screen.hpp"
#include "characters/character_roster.hpp"
#include "raylib.h"
#include <algorithm>
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
    int padding = 26;
    int fontSize = 26;

    // ── Bottom-left HUD ───────────────────────────────────────────────────────
    const int barWidth = 200;
    std::string healthText = std::to_string((int)m_localPlayer.health) + " HP";

    const int barHeight = 24;
    float panelW = barWidth + MeasureText(healthText.c_str(), fontSize) + (padding * 2);
    float panelH = fontSize + (barHeight + padding * 2);
    float panelX = padding - 6.0f;
    float panelY = screenH - panelH - padding + 2.0f;

    Rectangle panelRect = {panelX, panelY, panelW, panelH};
    DrawRectangleRounded(panelRect, 0.3f, 8, Fade(BLACK, 0.5f));

    RenderHealthBar(panelRect, healthText, padding, barHeight, barWidth, fontSize);
    RenderWalls(panelRect, padding, barHeight, fontSize);

    // ── Kill feed (above HUD, left side) ─────────────────────────────────────
    int feedFontSize = 18;
    int feedLineH = feedFontSize + 6; // line height with a little breathing room
    int feedBaseY = screenH - (barHeight * 2) - (padding * 3) - feedLineH;

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

void HudScreen::RenderHealthBar(const Rectangle &container, const std::string &healthText, int padding, int barHeight,
                                int barWidth, int fontSize) {
    float maxHealth = Character::GetCharacterDef(m_localPlayer.characterId).maxHealth;
    float hpPercent = std::clamp(m_localPlayer.health / maxHealth, 0.0f, 1.0f);

    Color hpColor;

    if (hpPercent > 0.5f) {
        // Green -> Yellow
        float t = (hpPercent - 0.5f) / 0.5f;
        hpColor = Color{255, 255, 0, 255};

        hpColor.r = (unsigned char)(255 * (1.0f - t));
        hpColor.g = 255;
    } else {
        // Yellow -> Red
        float t = hpPercent / 0.5f;

        hpColor = Color{255, (unsigned char)(255 * t), 0, 255};
    }

    int barX = container.x + padding;
    int barY = container.y + padding - ((float)barHeight / 2);

    // Background
    DrawRectangleRounded(Rectangle{(float)barX, (float)barY, (float)barWidth, (float)barHeight}, 0.4f, 8,
                         Fade(BLACK, 0.6f));

    // Fill
    DrawRectangleRounded(Rectangle{(float)barX, (float)barY, barWidth * hpPercent, (float)barHeight}, 0.4f, 8, hpColor);

    DrawText(healthText.c_str(), barX + barWidth + 10, barY + 1, fontSize, RAYWHITE);
}

void HudScreen::RenderWalls(const Rectangle &container, int padding, int barHeight, int fontSize) {
    int maxWalls = Character::GetCharacterDef(m_localPlayer.characterId).maxWalls;
    const int wallSize = 18;
    const int wallGap = 6;

    int wallsX = container.x + padding;
    int barY = container.y + container.height - padding - ((float)barHeight / 2);

    for (int i = 0; i < maxWalls; i++) {
        bool available = i < m_localPlayer.currentAvaliableWalls;

        Color wallColor = available ? SKYBLUE : Fade(GRAY, 0.4f);

        DrawRectangleRounded(
            Rectangle{(float)(wallsX + i * (wallSize + wallGap)), (float)barY, (float)wallSize, (float)wallSize}, 0.2f,
            4, wallColor);

        DrawRectangleLinesEx(
            Rectangle{(float)(wallsX + i * (wallSize + wallGap)), (float)barY, (float)wallSize, (float)wallSize}, 2.0f,
            Fade(WHITE, 0.5f));
    }

    // Walls text
    std::string wallsText = std::to_string((int)m_localPlayer.currentAvaliableWalls) + "/" + std::to_string(maxWalls);

    DrawText(wallsText.c_str(), wallsX + (maxWalls * (wallSize + wallGap)) + 10, barY, fontSize, RAYWHITE);
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
