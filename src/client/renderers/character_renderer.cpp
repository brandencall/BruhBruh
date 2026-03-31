#include "character_renderer.hpp"
#include "raymath.h"
#include <cstdint>

namespace Render {

void CharacterRenderer::Load() {
    m_textures[Character::CharacterId::Tonts] = LoadTexture("assets/characters/tmp/Tonts.png");
    m_textures[Character::CharacterId::Raff] = LoadTexture("assets/characters/tmp/Chavz.png");
    m_textures[Character::CharacterId::Hodge] = LoadTexture("assets/characters/tmp/Hodge.png");
    m_textures[Character::CharacterId::JJ] = LoadTexture("assets/characters/tmp/Big_J.png");
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

    float smoothing = 15.0f; // lower is smoother, higher is snappier
    Vector2 &current = m_positions[state.id];
    current = Vector2Lerp(current, state.position, smoothing * dt);
}

void CharacterRenderer::Draw(const state::PlayerState &player) {
    auto it = m_textures.find(player.characterId);
    if (it == m_textures.end())
        return;
    // Draw centered on player position
    DrawTextureV(
        it->second,
        {m_positions[player.id].x - it->second.width * 0.5f, m_positions[player.id].y - it->second.height * 0.5f},
        WHITE);
}

void CharacterRenderer::SnapToPosition(const state::PlayerState &state) {
    m_positions[state.id] = {state.position.x, state.position.y};
}

Vector2 CharacterRenderer::GetPosition(uint32_t playerId) { return m_positions[playerId]; }

} // namespace Render
