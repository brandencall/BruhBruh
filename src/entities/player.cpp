#include "player.hpp"
#include "collision_components.hpp"
#include "entity.hpp"
#include <iostream>

// Collider and hurtbox have the same AABB values, may just want them to share that in the future
Player::Player(Vector2 startPos)
    : m_position(startPos), m_health(100),
      m_collider(new Component::Collider({m_position.x, m_position.y, m_width, m_height}, false, this)),
      m_hurtbox(new Component::Hurtbox({m_position.x, m_position.y, m_width, m_height}, this)) {}

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
    m_hurtbox->bounds.x = m_position.x;
    m_hurtbox->bounds.y = m_position.y;
}

void Player::Draw() { DrawRectangleV(m_position, {m_width, m_height}, BLUE); }

Vector2 Player::GetPosition() const { return m_position; }

void Player::OnCollision(Entity *entity) {
    std::cout << "Player collided with entity at " << entity->position.x << ", " << entity->position.y << "\n";
}

void Player::ApplyDamage(int damage) {
    m_health.takeDamage(damage);
    std::cout << "Applying damage to player: " << damage << std::endl;
    std::cout << "Player health after damage: " << m_health.getCurrentHealth() << std::endl;
}
