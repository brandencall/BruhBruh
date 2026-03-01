#pragma once
#include "../components/health_component.hpp"
#include "collision_components.hpp"
#include "entity.hpp"
#include "iostream"
#include <memory>

// TESTING Entity dmg
class TestEntity : public Entity {
  public:
    bool m_isDead = false;
    float width;
    float height;
    std::unique_ptr<Component::HealthComponent> m_health;
    std::unique_ptr<Component::Hurtbox> m_hurtbox;

    TestEntity(float x, float y, float w, float h) : m_health(std::make_unique<Component::HealthComponent>(100)) {
        position = {x, y};
        width = w;
        height = h;

        m_hurtbox = std::make_unique<Component::Hurtbox>(Component::AABB{x, y, w, h}, this);
    }

    void Update(float dt) override {}
    bool IsDead() override { return m_isDead; }

    void SetDead() override { m_isDead = true; }

    void Draw() override { DrawRectangle(position.x, position.y, width, height, GREEN); }

    EntityType GetType() const override { return EntityType::Player; }
    Component::Hurtbox *GetHurtbox() const override { return m_hurtbox.get(); }

    Vector2 GetPosition() const override { return position; }

    void ApplyDamage(int damage) override {
        m_health->takeDamage(damage);
        if (m_health->getCurrentHealth() <= 0) {
            m_isDead = true;
        }
        std::cout << "Applying damage to test: " << damage << std::endl;
        std::cout << "Test health after damage: " << m_health->getCurrentHealth() << std::endl;
    }
};
