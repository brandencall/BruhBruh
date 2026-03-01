#include "bullet_entity.hpp"
#include "collider.hpp"
#include "collision_components.hpp"
#include "raymath.h"

BulletEntity::BulletEntity(Vector2 startPos, Vector2 direction, Entity *shooter)
    : m_position(startPos), m_direction(Vector2Normalize(direction)),
      m_collider(new Component::Collider(Component::AABB{m_position.x, m_position.y, m_size, m_size}, false, this)),
      m_hitbox(new Component::Hitbox(Component::AABB{m_position.x, m_position.y, m_size, m_size}, 10, this, shooter)) {}

BulletEntity::EntityType BulletEntity::GetType() const { return EntityType::Bullet; }

void BulletEntity::Update(float dt) {
    m_position += m_direction * m_speed * dt;
    m_lifetime -= dt;

    m_collider->bounds.x = m_position.x;
    m_collider->bounds.y = m_position.y;
    m_hitbox->bounds.x = m_position.x;
    m_hitbox->bounds.y = m_position.y;
}

void BulletEntity::Draw() { DrawRectangleV(m_position, {m_size, m_size}, RED); }

Vector2 BulletEntity::GetPosition() const { return m_position; }

Component::Collider *BulletEntity::GetCollider() const { return m_collider.get(); }

bool BulletEntity::IsDead() {
    if (m_lifetime <= 0.0f) {
        m_isDead = true;
    }
    return m_isDead;
}

void BulletEntity::SetDead() { m_isDead = true; }

Component::Hitbox *BulletEntity::GetHitbox() const { return m_hitbox.get(); }

void BulletEntity::OnCollision(Entity *entity) {
    if (entity == m_hitbox->shooter || entity->GetType() == EntityType::Bullet)
        return;

    m_isDead = true;
}
