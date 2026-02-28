#pragma once
#include "../components/collider.hpp"
#include "../components/health_component.hpp"
#include "collision_components.hpp"
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
    void ApplyDamage(int damage) override;

  private:
    float m_speed = 200.0f;
    float m_width = 32.0f;
    float m_height = 32.0f;
    Vector2 m_position;
    Component::HealthComponent m_health;

  public:
    std::unique_ptr<Component::Collider> m_collider;
    std::unique_ptr<Component::Hurtbox> m_hurtbox;
};
