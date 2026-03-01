#pragma once
#include "../components/collider.hpp"
#include "collision_components.hpp"
#include "entity.hpp"
#include <memory>

// TESTING COLLISION
class Wall : public Entity {
  public:
    float width;
    float height;
    std::unique_ptr<Component::Collider> m_collider;

    Wall(float x, float y, float w, float h) {
        position = {x, y};
        width = w;
        height = h;

        m_collider = std::make_unique<Component::Collider>(Component::AABB{x, y, w, h}, true, this);
    }

    EntityType GetType() const override { return EntityType::Wall; }
    Component::Collider *GetCollider() const override { return m_collider.get(); }
    void Update(float dt) override {}

    void Draw() override { DrawRectangle(position.x, position.y, width, height, BLUE); }
    Vector2 GetPosition() const override { return position; }
};
