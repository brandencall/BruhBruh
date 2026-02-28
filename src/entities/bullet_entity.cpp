#include "bullet_entity.hpp"
#include "collision_components.hpp"
#include "raymath.h"

BulletEntity::BulletEntity(Vector2 startPos, Vector2 direction, Entity *shooter)
    : m_position(startPos), m_direction(Vector2Normalize(direction)),
      m_hitbox(new Component::Hitbox(Component::AABB{m_position.x, m_position.y, m_size, m_size}, 10, this, shooter)) {}

void BulletEntity::Update(float dt) {
    m_position += m_direction * m_speed * dt;
    m_lifetime -= dt;
    m_hitbox->bounds.x = m_position.x;
    m_hitbox->bounds.y = m_position.y;
}

void BulletEntity::Draw() { DrawRectangleV(m_position, {m_size, m_size}, RED); }

Vector2 BulletEntity::GetPosition() const { return m_position; }

bool BulletEntity::IsDead() const { return m_lifetime <= 0.0f; }

Component::Hitbox *BulletEntity::getHitbox() { return m_hitbox.get(); }
