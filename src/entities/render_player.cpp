#include "render_player.hpp"
#include "player_state.hpp"
#include "raymath.h"

RenderPlayer::RenderPlayer(uint32_t id) : m_id(id) {}

void RenderPlayer::Sync(const PlayerState &state, float dt) {
    float smoothing = 10.0f;
    m_position = Vector2Lerp(m_position, state.position, dt * smoothing);
    m_active = state.active;
}

void RenderPlayer::Draw() { DrawRectangleV(m_position, {m_width, m_height}, BLUE); }

Vector2 RenderPlayer::GetPosition() const { return m_position; }
