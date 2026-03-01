#pragma once
#include "collider.hpp"
#include "collision_components.hpp"
#include <raylib.h>

class Entity {
  public:
    enum class EntityType { Player, Bullet, Wall };
    virtual ~Entity() = default;

    virtual void Update(float dt) = 0;
    virtual void Draw() = 0;

    virtual Vector2 GetPosition() const = 0;
    virtual EntityType GetType() const = 0;
    virtual void OnCollision(Entity *other) {}
    virtual void ApplyDamage(int damage) {}
    virtual bool IsDead() { return false; }
    virtual void SetDead() {}
    virtual Component::Hitbox *GetHitbox() const { return nullptr; }
    virtual Component::Hurtbox *GetHurtbox() const { return nullptr; }
    virtual Component::Collider *GetCollider() const { return nullptr; }

  public:
    Vector2 position;
};
