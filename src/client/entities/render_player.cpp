#include "render_player.hpp"
#include "raymath.h"

RenderPlayer::RenderPlayer(uint32_t id) : m_id(id) {}

void RenderPlayer::Sync(const state::PlayerState &state, float dt) {
    float smoothing = 10.0f;
    m_position = Vector2Lerp(m_position, state.position, dt * smoothing);
    m_active = state.active;
    m_radius = state.hurtbox.radius;
}

void RenderPlayer::Draw() { DrawCircleV(m_position, m_radius, BLUE); }

Vector2 RenderPlayer::GetPosition() const { return m_position; }
