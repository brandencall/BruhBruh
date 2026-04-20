#include "character_renderer.hpp"
#include "raymath.h"
#include <cstdint>

namespace Render {

void CharacterRenderer::Load() {
    m_textures[Character::CharacterId::Tonts] = LoadTexture("assets/characters/Tonts-Sheet.png");
    m_textures[Character::CharacterId::Raff] = LoadTexture("assets/characters/Chavz-Sheet.png");
    m_textures[Character::CharacterId::Hodge] = LoadTexture("assets/characters/Hodges-Sheet.png");
    m_textures[Character::CharacterId::JJ] = LoadTexture("assets/characters/Jontiy-Sheet.png");
}

void CharacterRenderer::Unload() {
    for (auto &[id, tex] : m_textures)
        UnloadTexture(tex);
}

void CharacterRenderer::Sync(const state::PlayerState &state, float dt) {
    // If this is a new player, snap immediately instead of lerping from origin
    if (m_positions.find(state.id) == m_positions.end()) {
        m_positions[state.id] = state.position;
        return;
    }

    float smoothing = 10.0f; // lower is smoother, higher is snappier
    Vector2 &current = m_positions[state.id];
    current = Vector2Lerp(current, state.position, smoothing * dt);
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

    DrawTexturePro(tex, src, dst, origin, 0.0f, WHITE);
}

void CharacterRenderer::SnapToPosition(const state::PlayerState &state) {
    m_positions[state.id] = {state.position.x, state.position.y};
}

Vector2 CharacterRenderer::GetPosition(uint32_t playerId) { return m_positions[playerId]; }

} // namespace Render
