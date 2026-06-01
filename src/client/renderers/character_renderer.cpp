#include "character_renderer.hpp"
#include "../../shared/characters/character_roster.hpp"
#include "raylib.h"
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

        Draw(player);
        // DebugHitBox(player);
    }
}

void CharacterRenderer::DebugHitBox(const state::PlayerState &player) {
    Vector2 hurtboxCenter = {player.position.x + player.hurtbox.offsetX, player.position.y + player.hurtbox.offsetY};
    DrawCircleV(hurtboxCenter, player.hurtbox.radius, {255, 0, 0, 80});
    DrawCircleLinesV(hurtboxCenter, player.hurtbox.radius, RED);
}

void CharacterRenderer::Draw(const state::PlayerState &player) {
    auto it = m_textures.find(player.characterId);
    if (it == m_textures.end())
        return;

    Texture2D &tex = it->second;
    int texRow = 0;
    float angle = player.currentInput.angle; // radians
                                             // Normalize to 0 → 2π
    if (angle < 0)
        angle += 2 * PI;

    int direction = (int)round(angle / (2 * PI) * DIR_COUNT) % DIR_COUNT;
    Color base = WHITE;

    switch (player.state) {
    case state::State::Idle:
        texRow = IDLE_ROW_OFFSET + direction;
        break;
    case state::State::Running:
        texRow = RUN_ROW_OFFSET + direction;
        break;
    case state::State::Dead:
        texRow = DEAD_ROW;
        break;
    }

    int frameWidth = tex.width / FRAME_COUNT;
    int frameHeight = tex.height / ROW_COUNT;

    // Vector2 position = m_positions[player.id];
    Vector2 position = player.position;
    // TODO: pull frame from player instead of hard coding the 1st frame
    int frame = 0;
    Rectangle src = {(float)frame * frameWidth, (float)texRow * frameHeight, (float)frameWidth, (float)frameHeight};
    Rectangle dst = {position.x, position.y, (float)frameWidth, (float)frameHeight};

    Vector2 origin = {frameWidth * 0.5f, frameHeight * 0.5f};

    if (player.invincibilityTimer > 0.0f) {
        float blinkTimer = m_blinkTimers[player.id];
        float t = sinf(blinkTimer * 20.0f) * 0.5f + 0.5f;
        base =
            Color{(unsigned char)(255 * (0.5f + 0.5f * (1.0f - t))), (unsigned char)(255 * (0.5f + 0.5f * (1.0f - t))),
                  (unsigned char)(255 * (0.5f + 0.5f * (1.0f - t))), 255};
    }

    DrawTexturePro(tex, src, dst, origin, 0.0f, base);
    DrawHealthBar(player, position, frameWidth, frameHeight);
    DrawPlayerAura(player);
}

void CharacterRenderer::DrawHealthBar(const state::PlayerState &player, Vector2 position, int frameWidth,
                                      int frameHeight) {
    float healthPct = player.health / Character::GetCharacterDef(player.characterId).maxHealth;
    float healthBarPosX = position.x - (float)frameWidth / 2;
    float healthBarPosY = (position.y - (float)frameHeight / 2) - 4;
    DrawRectangle(healthBarPosX, healthBarPosY, frameWidth * healthPct, 4, GREEN);
    DrawRectangleLines(healthBarPosX, healthBarPosY, frameWidth, 4, BLACK);
}

void CharacterRenderer::DrawPlayerAura(const state::PlayerState &player) {
    // Vector2 renderPos = GetPosition(player.id);
    // Vector2 center = {renderPos.x + player.hurtbox.offsetX, renderPos.y + player.hurtbox.offsetY};
    Vector2 center = {player.position.x + player.hurtbox.offsetX, player.position.y + player.hurtbox.offsetY};

    const float BASE_RADIUS = player.hurtbox.radius + 4.0f; // just outside the hurtbox
    const float RING_STEP = 6.0f;
    const float RING_THICK = 3.0f;

    int ringIndex = 0;
    for (const auto &effect : player.effects) {
        if (!effect.active)
            continue;

        const state::EffectDefinition &def = GetEffectDefinition(effect.type);

        float ratio = effect.durationRemaining / effect.maxDuration;
        float fade = 0.4f + 0.6f * ratio;

        Color c = def.color;
        c.a = static_cast<uint8_t>(c.a * fade);

        float radius = BASE_RADIUS + (ringIndex * RING_STEP);
        DrawRing(center, radius, radius + RING_THICK, 0, 360, 32, c);

        ringIndex++;
    }
}

} // namespace Render
