#include "player.hpp"

Player::Player(Vector2 startPos) : m_position(startPos) {}

void Player::Update(float dt) {
    if (IsKeyDown(KEY_W))
        m_position.y -= m_speed * dt;
    if (IsKeyDown(KEY_S))
        m_position.y += m_speed * dt;
    if (IsKeyDown(KEY_A))
        m_position.x -= m_speed * dt;
    if (IsKeyDown(KEY_D))
        m_position.x += m_speed * dt;
}

void Player::Draw() { DrawRectangleV(m_position, {32, 32}, BLUE); }

Vector2 Player::GetPosition() const { return m_position; }
