#include "render_player.hpp"
#include "player_state.hpp"

RenderPlayer::RenderPlayer(uint32_t id) : m_id(id) {}

void RenderPlayer::Sync(const PlayerState &state) {
    m_position.x = state.position.x;
    m_position.y = state.position.y;
    m_active = state.active;
}

void RenderPlayer::Draw() { DrawRectangleV(m_position, {m_width, m_height}, BLUE); }

Vector2 RenderPlayer::GetPosition() const { return m_position; }
