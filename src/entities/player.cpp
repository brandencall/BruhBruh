#include "player.hpp"
#include "entity.hpp"
#include <iostream>

Player::Player(Vector2 startPos)
    : m_position(startPos), m_collider(new Collider({m_position.x, m_position.y, m_width, m_height}, false)) {
    m_collider->owner = this;
}

void Player::Update(float dt) {
    if (IsKeyDown(KEY_W))
        m_position.y -= m_speed * dt;
    if (IsKeyDown(KEY_S))
        m_position.y += m_speed * dt;
    if (IsKeyDown(KEY_A))
        m_position.x -= m_speed * dt;
    if (IsKeyDown(KEY_D))
        m_position.x += m_speed * dt;

    m_collider->bounds.x = m_position.x;
    m_collider->bounds.y = m_position.y;
}

void Player::Draw() { DrawRectangleV(m_position, {m_width, m_height}, BLUE); }

Vector2 Player::GetPosition() const { return m_position; }

void Player::OnCollision(Entity *entity) {

    std::cout << "Player collided with entity at " << entity->position.x << ", " << entity->position.y << "\n";
}
