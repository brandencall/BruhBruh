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
    std::unique_ptr<Component::Hitbox> m_hitbox;

    Wall(float x, float y, float w, float h) {
        position = {x, y};
        width = w;
        height = h;

        m_collider = std::make_unique<Component::Collider>(Component::AABB{x, y, w, h}, true, this);
        m_hitbox = std::make_unique<Component::Hitbox>(Component::AABB{x, y, w, h}, 10, this);
    }

    void Update(float dt) override {}

    void Draw() override { DrawRectangle(position.x, position.y, width, height, BLUE); }
    Vector2 GetPosition() const override { return position; }
};
