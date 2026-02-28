#pragma once
#include "../components/health_component.hpp"
#include "collision_components.hpp"
#include "entity.hpp"
#include "iostream"
#include <memory>

// TESTING Entity dmg
class TestEntity : public Entity {
  public:
    float width;
    float height;
    Component::HealthComponent m_health;
    std::unique_ptr<Component::Hurtbox> m_hurtbox;

    TestEntity(float x, float y, float w, float h) : m_health(100) {
        position = {x, y};
        width = w;
        height = h;

        m_hurtbox = std::make_unique<Component::Hurtbox>(Component::AABB{x, y, w, h}, this);
    }

    void Update(float dt) override {}

    void Draw() override { DrawRectangle(position.x, position.y, width, height, GREEN); }

    Vector2 GetPosition() const override { return position; }

    void ApplyDamage(int damage) override {
        m_health.takeDamage(damage);
        std::cout << "Applying damage to test: " << damage << std::endl;
        std::cout << "Test health after damage: " << m_health.getCurrentHealth() << std::endl;
    }
};
