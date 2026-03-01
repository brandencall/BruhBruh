#pragma once
#include "collider.hpp"
#include "collision_components.hpp"
#include "entity.hpp"
#include <memory>
#include <raylib.h>
#include <raymath.h>

class BulletEntity : public Entity {
  public:
    BulletEntity(Vector2 startPos, Vector2 direction, Entity *shooter);

    void Update(float dt) override;
    void Draw() override;

    EntityType GetType() const override;
    Component::Collider *GetCollider() const override;
    Vector2 GetPosition() const override;
    bool IsDead() override;
    void SetDead() override;
    void OnCollision(Entity *entity) override;
    Component::Hitbox *GetHitbox() const override;

  private:
    bool m_isDead = false;
    float m_speed = 300.0f;
    float m_lifetime = 2.0f;
    float m_size = 16.0f;
    Vector2 m_position;
    Vector2 m_direction;

  public:
    std::unique_ptr<Component::Collider> m_collider;
    std::unique_ptr<Component::Hitbox> m_hitbox;
};
