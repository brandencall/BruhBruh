#include "bullet_entity.hpp"
#include "raymath.h"

BulletEntity::BulletEntity(Vector2 startPos, Vector2 direction)
    : m_position(startPos), m_direction(Vector2Normalize(direction)) {}

void BulletEntity::Update(float dt) {
    m_position += m_direction * m_speed * dt;
    m_lifetime -= dt;
}

void BulletEntity::Draw() { DrawRectangleV(m_position, {16, 16}, RED); }

Vector2 BulletEntity::GetPosition() const { return m_position; }

bool BulletEntity::IsDead() const { return m_lifetime <= 0.0f; }
