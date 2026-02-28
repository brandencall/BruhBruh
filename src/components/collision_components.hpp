#pragma once
#include "entity.hpp"

namespace Component {
struct AABB {
    float x;
    float y;
    float width;
    float height;
    bool Overlaps(const AABB &other) const {
        return (x < other.x + other.width && x + width > other.x && y < other.y + other.height && y + height > other.y);
    }
};

struct Hurtbox {
    AABB bounds;
    Entity *owner;
};

struct Hitbox {
    AABB bounds;
    int damage;
    Entity *owner;
    Entity *shooter;
};
} // namespace Component
