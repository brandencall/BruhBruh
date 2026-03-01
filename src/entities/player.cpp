#include "player.hpp"
#include "collider.hpp"
#include "collision_components.hpp"
#include "entity.hpp"
#include "raymath.h"
#include <iostream>

// Collider and hurtbox have the same AABB values, may just want them to share that in the future
Player::Player(Vector2 startPos)
    : m_position(startPos), m_health(100),
      m_collider(new Component::Collider({m_position.x, m_position.y, m_width, m_height}, false, this)),
      m_hurtbox(new Component::Hurtbox({m_position.x, m_position.y, m_width, m_height}, this)) {}

void Player::Update(float dt) {
    Vector2 dir = {0.0f, 0.0f};

    if (IsKeyDown(KEY_W))
        dir.y -= 1.0f;
    if (IsKeyDown(KEY_S))
        dir.y += 1.0f;
    if (IsKeyDown(KEY_A))
        dir.x -= 1.0f;
    if (IsKeyDown(KEY_D))
        dir.x += 1.0f;

    dir = Vector2Normalize(dir);

    m_position.x += dir.x * m_speed * dt;
    m_position.y += dir.y * m_speed * dt;

    m_collider->bounds.x = m_position.x;
    m_collider->bounds.y = m_position.y;
    m_hurtbox->bounds.x = m_position.x;
    m_hurtbox->bounds.y = m_position.y;
}

void Player::Draw() { DrawRectangleV(m_position, {m_width, m_height}, BLUE); }

Player::EntityType Player::GetType() const { return EntityType::Player; }

Component::Collider *Player::GetCollider() const { return m_collider.get(); }

Vector2 Player::GetPosition() const { return m_position; }

bool Player::IsDead() { return m_isDead; }

void Player::SetDead() { m_isDead = true; }

Component::Hurtbox *Player::GetHurtbox() const { return m_hurtbox.get(); }

void Player::OnCollision(Entity *entity) {
    if (entity->GetType() == EntityType::Bullet) {
        auto hitbox = entity->GetHitbox();
        if (hitbox && hitbox->shooter == this) {
            return;
        }
    }
    std::cout << "Player collided with entity at " << entity->position.x << ", " << entity->position.y << "\n";
}

void Player::ApplyDamage(int damage) {
    m_health.takeDamage(damage);
    std::cout << "Applying damage to player: " << damage << std::endl;
    std::cout << "Player health after damage: " << m_health.getCurrentHealth() << std::endl;
}
