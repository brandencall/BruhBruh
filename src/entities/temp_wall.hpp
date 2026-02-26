#pragma once
#include "../components/collider.hpp"
#include "entity.hpp"
#include <memory>

// TESTING COLLISION
class Wall : public Entity {
  public:
    float width;
    float height;
    std::unique_ptr<Collider> m_collider;

    Wall(float x, float y, float w, float h) {
        position = {x, y};
        width = w;
        height = h;

        m_collider = std::make_unique<Collider>(AABB{x, y, w, h}, true);
        m_collider->owner = this;
    }

    void Update(float dt) override {}

    void Draw() override { DrawRectangle(position.x, position.y, width, height, BLUE); }
    Vector2 GetPosition() const override { return position; }
};
