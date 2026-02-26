#pragma once
#include <raylib.h>
#include <raymath.h>

class BulletEntity {
  public:
    BulletEntity(Vector2 startPos, Vector2 direction);

    void Update(float dt);
    void Draw();

    Vector2 GetPosition() const;
    bool IsDead() const;

  private:
    float m_speed = 300.0f;
    float m_lifetime = 2.0f;
    Vector2 m_position;
    Vector2 m_direction;
};
