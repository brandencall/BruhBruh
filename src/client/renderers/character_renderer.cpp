#include "character_renderer.hpp"
#include "../../shared/characters/character_roster.hpp"
#include "raylib.h"
#include "raymath.h"
#include <array>
#include <cstdint>

namespace Render {

void CharacterRenderer::Load() {
    m_textures[Character::CharacterId::Tonts] = LoadTexture("assets/characters/Tonts-Sheet.png");
    m_textures[Character::CharacterId::Raff] = LoadTexture("assets/characters/Chavz-Sheet.png");
    m_textures[Character::CharacterId::Hodge] = LoadTexture("assets/characters/Hodges-Sheet.png");
    m_textures[Character::CharacterId::JJ] = LoadTexture("assets/characters/Jontiy-Sheet.png");

    for (const auto &tex : m_textures) {
        SetTextureFilter(tex.second, TEXTURE_FILTER_POINT);
    }
}

void CharacterRenderer::Unload() {
    for (auto &[id, tex] : m_textures)
        UnloadTexture(tex);
}

void CharacterRenderer::Sync(const state::PlayerState &state, float dt) {
    // If this is a new player, snap immediately instead of lerping from origin
    if (m_positions.find(state.id) == m_positions.end()) {
        m_positions[state.id] = state.position;
        m_blinkTimers[state.id] = 0.0f;
        return;
    }

    float smoothing = 10.0f; // lower is smoother, higher is snappier
    Vector2 &current = m_positions[state.id];
    current = Vector2Lerp(current, state.position, smoothing * dt);

    if (state.invincibilityTimer > 0.0f) {
        m_blinkTimers[state.id] += dt;
    } else {
        m_blinkTimers[state.id] = 0.0f;
    }
}

void CharacterRenderer::Draw(const std::array<state::PlayerState, MAX_PLAYERS> &players) {
    for (const auto &player : players) {
        if (!player.active)
            continue;

        // TODO: Instead of not drawing the dead player, draw the dead players death frames
        if (player.respawnTimer > 0.0f)
            continue;

        Draw(player);
    }
}

void CharacterRenderer::DebugHitBox(const state::PlayerState &player) {
    // Use lerped position so hurtbox stays on the sprite
    Vector2 renderPos = GetPosition(player.id);
    Vector2 hurtboxCenter = {renderPos.x + player.hurtbox.offsetX, renderPos.y + player.hurtbox.offsetY};
    DrawCircleV(hurtboxCenter, player.hurtbox.radius, {255, 0, 0, 80});
    DrawCircleLinesV(hurtboxCenter, player.hurtbox.radius, RED);
}

void CharacterRenderer::Draw(const state::PlayerState &player) {
    auto it = m_textures.find(player.characterId);
    if (it == m_textures.end())
        return;

    Texture2D &tex = it->second;

    float angle = player.currentInput.angle; // radians
    // Normalize to 0 → 2π
    if (angle < 0)
        angle += 2 * PI;

    int direction = (int)round(angle / (2 * PI) * m_numberOfDirections) % m_numberOfDirections;

    int frameWidth = tex.width / m_numberOfFrames;
    int frameHeight = tex.height / m_numberOfDirections;

    Vector2 position = m_positions[player.id];
    // TODO: pull frame from player instead of hard coding the 1st frame
    int frame = 0;
    Rectangle src = {(float)frame * frameWidth, (float)direction * frameHeight, (float)frameWidth, (float)frameHeight};
    Rectangle dst = {position.x, position.y, (float)frameWidth, (float)frameHeight};

    Vector2 origin = {frameWidth * 0.5f, frameHeight * 0.5f};

    Color base = WHITE;

    if (player.invincibilityTimer > 0.0f) {
        float blinkTimer = m_blinkTimers[player.id];
        float t = sinf(blinkTimer * 20.0f) * 0.5f + 0.5f;

        base =
            Color{(unsigned char)(255 * (0.5f + 0.5f * (1.0f - t))), (unsigned char)(255 * (0.5f + 0.5f * (1.0f - t))),
                  (unsigned char)(255 * (0.5f + 0.5f * (1.0f - t))), 255};
    }

    DrawTexturePro(tex, src, dst, origin, 0.0f, base);

    DrawHealthBar(player, position, frameWidth, frameHeight);
}

void CharacterRenderer::DrawHealthBar(const state::PlayerState &player, Vector2 position, int frameWidth,
                                      int frameHeight) {
    float healthPct = player.health / Character::GetCharacterDef(player.characterId).maxHealth;
    float healthBarPosX = position.x - (float)frameWidth / 2;
    float healthBarPosY = (position.y - (float)frameHeight / 2) - 4;
    DrawRectangle(healthBarPosX, healthBarPosY, frameWidth * healthPct, 4, GREEN);
    DrawRectangleLines(healthBarPosX, healthBarPosY, frameWidth, 4, BLACK);
}

void CharacterRenderer::SnapToPosition(const state::PlayerState &state) {
    m_positions[state.id] = {state.position.x, state.position.y};
}

Vector2 CharacterRenderer::GetPosition(uint32_t playerId) { return m_positions[playerId]; }

} // namespace Render
