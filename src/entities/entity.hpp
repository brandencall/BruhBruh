#pragma once
#include <raylib.h>

class Entity {
  public:
    virtual ~Entity() = default;

    virtual void Update(float dt) = 0;
    virtual void Draw() = 0;

    virtual Vector2 GetPosition() const = 0;
    virtual void OnCollision(Entity *other) {}
    virtual void ApplyDamage(int damage) {}

  public:
    Vector2 position;
};
