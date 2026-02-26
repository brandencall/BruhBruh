#pragma once
#include "../components/collider.hpp"
#include "entity.hpp"
#include <memory>
#include <raylib.h>

class Player : public Entity {
  public:
    Player(Vector2 startPos);

    void Update(float dt) override;
    void Draw() override;

    Vector2 GetPosition() const override;
    void OnCollision(Entity *entity) override;

  private:
    float m_speed = 200.0f;
    float m_width = 32.0f;
    float m_height = 32.0f;
    Vector2 m_position;

  public:
    std::unique_ptr<Collider> m_collider;
};
