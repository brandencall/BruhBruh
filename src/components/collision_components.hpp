#pragma once

class Entity;

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

// Might be able to delete the owner
struct Hurtbox {
    AABB bounds;
    Entity *owner;
};

// Might be able to delete the owner/shooter
struct Hitbox {
    AABB bounds;
    int damage;
    Entity *owner;
    Entity *shooter;
};
} // namespace Component
