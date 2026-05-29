#include "hud_screen.hpp"
#include "../../../shared/characters/character_roster.hpp"
#include "raylib.h"
#include <algorithm>
#include <cmath>
#include <string>

namespace UI {

// ── Internal helpers ──────────────────────────────────────────────────────────

// Draw a rounded-rect border only (no fill).
static void DrawRectangleRoundedBorder(Rectangle rec, float roundness, int segments, float thick, Color color) {
    DrawRectangleRoundedLinesEx(rec, roundness, segments, thick, color);
}

// Draw the icon with a top-to-bottom drain effect.
//   t = fraction of duration *remaining*  (1.0 = full, 0.0 = expired)
// The coloured portion shrinks from bottom upward as time runs out,
// leaving a dark/greyed-out top section — opposite to Overwatch's fill style.
static void DrawIconWithDrain(const Texture2D &tex, Rectangle dest, float t) {
    t = std::clamp(t, 0.0f, 1.0f);

    // Source rect for the full texture
    Rectangle src = {0, 0, (float)tex.width, (float)tex.height};

    // 1. Draw the full icon tinted dark (the "drained" / greyscale layer)
    DrawTexturePro(tex, src, dest, {0, 0}, 0.0f, Color{60, 60, 60, 220});

    // 2. Draw the coloured portion using a scissor rect.
    //    The coloured band starts at the bottom and its height = t * iconHeight.
    //    As t decreases the band shrinks upward, so colour drains top-to-bottom.
    if (t > 0.0f) {
        float colourH = dest.height * t;
        float colourY = dest.y + dest.height - colourH; // bottom-anchored

        // Scissor clip to the coloured band
        BeginScissorMode((int)dest.x, (int)colourY, (int)dest.width, (int)std::ceil(colourH));

        DrawTexturePro(tex, src, dest, {0, 0}, 0.0f, WHITE);

        EndScissorMode();
    }
}

// Slot accent colours — one set per logical category.
struct SlotTheme {
    Color bg;
    Color border; // used when not near expiry
};

static SlotTheme GetSlotTheme(bool isAbility, state::EffectCategory category) {
    if (isAbility)
        return {Color{20, 30, 50, 200}, Color{80, 140, 220, 160}}; // blue

    switch (category) {
    case state::EffectCategory::Buff:
        return {Color{15, 40, 20, 200}, Color{60, 200, 80, 170}}; // green
    case state::EffectCategory::Debuff:
        return {Color{45, 15, 15, 200}, Color{210, 60, 60, 170}}; // red
    default:
        return {Color{30, 20, 50, 200}, Color{160, 80, 220, 160}}; // purple fallback
    }
}

static void DrawSlotBackground(Rectangle slot, bool isAbility, state::EffectCategory category, bool nearExpiry) {
    SlotTheme theme = GetSlotTheme(isAbility, category);
    DrawRectangleRounded(slot, 0.25f, 8, theme.bg);

    Color borderColor;
    if (nearExpiry) {
        // Pulse between dark-red and bright-red regardless of category
        float pulse = 0.5f + 0.5f * sinf((float)GetTime() * 6.0f);
        unsigned char r = (unsigned char)(180 + 75 * pulse);
        borderColor = Color{r, 30, 30, 255};
    } else {
        borderColor = theme.border;
    }
    DrawRectangleRoundedBorder(slot, 0.25f, 8, 1.5f, borderColor);
}

// ── HudScreen ─────────────────────────────────────────────────────────────────

HudScreen::HudScreen(const state::PlayerState &localPlayer, const float &gameTime, Client::EventHub &events,
                     Render::AbilityRenderer abilityRenderer)
    : m_localPlayer(localPlayer), m_gameTime(gameTime), m_events(events), m_abilityRenderer(abilityRenderer) {
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

    // ── Active Effects + Abilities (bottom-right panel) ───────────────────────
    RenderPickupPanel(screenW, screenH, padding);

    // ── Kill feed (above HUD, left side) ─────────────────────────────────────
    int feedFontSize = 18;
    int feedLineH = feedFontSize + 6;
    int feedBaseY = screenH - (barHeight * 2) - (padding * 3) - feedLineH;

    const std::vector<System::Feed> feed = m_killFeed.GetFeed();

    for (int i = (int)feed.size() - 1; i >= 0; i--) {
        const auto &entry = feed[i];
        std::string line = entry.killer + " > " + entry.victim;

        float t = (feed.size() > 1) ? (float)(feed.size() - 1 - i) / (float)(feed.size() - 1) : 0.0f;
        unsigned char alpha = (unsigned char)(255 - t * 150);
        Color color = {255, 255, 255, alpha};

        DrawText(line.c_str(), padding, feedBaseY - ((int)feed.size() - 1 - i) * feedLineH, feedFontSize, color);
    }

    RenderGameTime(screenW);
}

// ── New: unified pickup panel ─────────────────────────────────────────────────

void HudScreen::RenderPickupPanel(int screenW, int screenH, int padding) {
    const auto &textures = m_abilityRenderer.GetTextures();

    constexpr int iconSize = 46; // slightly larger for breathing room
    constexpr int slotPad = 4;   // padding inside each slot
    constexpr int iconGap = 6;   // gap between slots
    constexpr int labelH = 14;   // height reserved for row label
    constexpr int labelFS = 11;  // font size for row labels
    constexpr int rowGap = 8;    // vertical gap between ability/effect rows
    constexpr int panelPadX = 10;
    constexpr int panelPadY = 8;
    constexpr float timerFS = 16;

    // Count active items in each category
    int numAbilities = 0;
    for (const auto &a : m_localPlayer.abilities)
        if (a.active)
            numAbilities++;

    int numEffects = 0;
    for (const auto &e : m_localPlayer.effects)
        if (e.active)
            numEffects++;

    if (numAbilities == 0 && numEffects == 0)
        return;

    int maxSlots = std::max(numAbilities, numEffects);
    int rowWidth = numAbilities > 0 ? numAbilities * iconSize + (numAbilities - 1) * iconGap
                                    : numEffects * iconSize + (numEffects - 1) * iconGap;
    // Panel width fits the widest row
    int abRow = numAbilities * iconSize + std::max(0, numAbilities - 1) * iconGap;
    int efRow = numEffects * iconSize + std::max(0, numEffects - 1) * iconGap;
    int innerW = std::max(abRow, efRow);

    int numRows = (numAbilities > 0 ? 1 : 0) + (numEffects > 0 ? 1 : 0);
    int innerH = numRows * (labelH + iconSize) + std::max(0, numRows - 1) * rowGap;

    float panelW = innerW + panelPadX * 2;
    float panelH = innerH + panelPadY * 2;
    float panelX = (screenW - panelW) / 2.0f;
    float panelY = screenH - padding - panelH;

    // Panel background
    DrawRectangleRounded({panelX, panelY, panelW, panelH}, 0.25f, 8, Color{10, 12, 20, 210});
    DrawRectangleRoundedBorder({panelX, panelY, panelW, panelH}, 0.25f, 8, 1.0f, Color{255, 255, 255, 30});

    float cursorY = panelY + panelPadY;

    // ── Abilities row ──────────────────────────────────────────────────────
    if (numAbilities > 0) {
        // Row label
        DrawText("ABILITIES", (int)(panelX + panelPadX - 4), (int)cursorY, labelFS, Color{80, 160, 255, 200});
        cursorY += labelH;

        float cursorX = panelX + panelPadX;

        for (const auto &ability : m_localPlayer.abilities) {
            if (!ability.active)
                continue;

            state::SpawnablePickup key{
                .pickupType = state::PickupType::Ability,
                .typeId = static_cast<uint8_t>(ability.type),
            };

            auto textureIt = textures.find(key);
            if (textureIt == textures.end()) {
                cursorX += iconSize + iconGap;
                continue;
            }

            Rectangle slotRect = {cursorX, cursorY, (float)iconSize, (float)iconSize};
            Rectangle iconRect = {cursorX + slotPad, cursorY + slotPad, (float)(iconSize - slotPad * 2),
                                  (float)(iconSize - slotPad * 2)};

            // Compute drain fraction: durationRemaining / totalDuration
            // We don't have totalDuration stored, so we approximate:
            // if durationRemaining > 0 it is still active; use it as-is normalised
            // against the character's ability duration (fallback: store max and use ratio).
            // For now we pass the raw remaining time clamped to [0,1] by dividing by a
            // reasonable max (e.g., 10 s). Adjust the divisor to match your actual max.
            float t = std::clamp(ability.durationRemaining / ability.maxDuration, 0.0f, 1.0f);
            bool expiry = ability.durationRemaining < 3.0f;

            DrawSlotBackground(slotRect, true, state::EffectCategory::None, expiry);
            DrawIconWithDrain(textureIt->second, iconRect, t);

            // Timer text centred below icon inside slot
            std::string timeText = TextFormat("%.1f", ability.durationRemaining);
            int tw = MeasureText(timeText.c_str(), (int)timerFS);
            Color timerColor = expiry ? Color{255, 100, 80, 255} : RAYWHITE;
            DrawText(timeText.c_str(), (int)(cursorX + (iconSize - tw) / 2),
                     (int)(cursorY + iconSize - (int)timerFS - 2), (int)timerFS, timerColor);

            cursorX += iconSize + iconGap;
        }

        cursorY += iconSize + rowGap;
    }

    // ── Effects row ───────────────────────────────────────────────────────
    if (numEffects > 0) {
        DrawText("EFFECTS", (int)(panelX + panelPadX - 4), (int)cursorY, labelFS, Color{200, 100, 255, 200});
        cursorY += labelH;

        float cursorX = panelX + panelPadX;

        for (const auto &effect : m_localPlayer.effects) {
            if (!effect.active)
                continue;

            state::SpawnablePickup key{
                .pickupType = state::PickupType::Effect,
                .typeId = static_cast<uint8_t>(effect.type),
            };

            auto textureIt = textures.find(key);
            if (textureIt == textures.end()) {
                cursorX += iconSize + iconGap;
                continue;
            }

            Rectangle slotRect = {cursorX, cursorY, (float)iconSize, (float)iconSize};
            Rectangle iconRect = {cursorX + slotPad, cursorY + slotPad, (float)(iconSize - slotPad * 2),
                                  (float)(iconSize - slotPad * 2)};

            float t = std::clamp(effect.durationRemaining / effect.maxDuration, 0.0f, 1.0f);
            bool expiry = effect.durationRemaining < 3.0f;

            DrawSlotBackground(slotRect, false, effect.category, expiry);
            DrawIconWithDrain(textureIt->second, iconRect, t);

            std::string timeText = TextFormat("%.1f", effect.durationRemaining);
            int tw = MeasureText(timeText.c_str(), (int)timerFS);
            Color timerColor = expiry ? Color{255, 100, 80, 255} : RAYWHITE;
            DrawText(timeText.c_str(), (int)(cursorX + (iconSize - tw) / 2),
                     (int)(cursorY + iconSize - (int)timerFS - 2), (int)timerFS, timerColor);

            cursorX += iconSize + iconGap;
        }
    }
}

// ── Existing methods (unchanged) ─────────────────────────────────────────────

void HudScreen::RenderHealthBar(const Rectangle &container, const std::string &healthText, int padding, int barHeight,
                                int barWidth, int fontSize) {
    float maxHealth = Character::GetCharacterDef(m_localPlayer.characterId).maxHealth;
    float hpPercent = std::clamp(m_localPlayer.health / maxHealth, 0.0f, 1.0f);

    Color hpColor;

    if (hpPercent > 0.5f) {
        float t = (hpPercent - 0.5f) / 0.5f;
        hpColor = Color{255, 255, 0, 255};
        hpColor.r = (unsigned char)(255 * (1.0f - t));
        hpColor.g = 255;
    } else {
        float t = hpPercent / 0.5f;
        hpColor = Color{255, (unsigned char)(255 * t), 0, 255};
    }

    int barX = container.x + padding;
    int barY = container.y + padding - ((float)barHeight / 2);

    DrawRectangleRounded(Rectangle{(float)barX, (float)barY, (float)barWidth, (float)barHeight}, 0.4f, 8,
                         Fade(BLACK, 0.6f));
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

    std::string wallsText = std::to_string((int)m_localPlayer.currentAvaliableWalls) + "/" + std::to_string(maxWalls);
    int wallIconsSize = wallsX + (maxWalls * (wallSize + wallGap)) + 10;
    DrawText(wallsText.c_str(), wallIconsSize, barY, fontSize, RAYWHITE);
    RenderWallCooldown(wallIconsSize + MeasureText(wallsText.c_str(), fontSize), barY, padding, fontSize, barHeight);
}

void HudScreen::RenderWallCooldown(int x, int y, int padding, int fontSize, int barHeight) {
    float wallCooldown = Character::GetCharacterDef(m_localPlayer.characterId).wallCooldown;
    float centerX = x + padding + 10.0f;
    float centerY = y + ((float)barHeight / 2);
    float radius = fontSize * 0.6f;
    float t = 1.0f - (m_localPlayer.wallTimer / wallCooldown);

    DrawCircleLines((int)centerX, (int)centerY, radius, Fade(RAYWHITE, 0.2f));
    DrawRing({centerX, centerY}, radius - 2.0f, radius, -90.0f, -90.0f + (360.0f * t), 32, t >= 1.0f ? GREEN : SKYBLUE);
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
