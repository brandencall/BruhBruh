#pragma once
#include "../state/player_state.hpp"
#include "raylib.h"
#include "raymath.h"
#include <algorithm>
#include <vector>

namespace Collision {

struct AABB {
    Vector2 min;
    Vector2 max;
};

struct Circle {
    Vector2 center;
    float radius;
};

inline bool Overlap(Circle a, Circle b) {
    float dx = a.center.x - b.center.x, dy = a.center.y - b.center.y;
    float r = a.radius + b.radius;
    return dx * dx + dy * dy <= r * r;
}

inline bool Overlap(Circle circle, AABB aabb) {
    Vector2 closest = {Clamp(circle.center.x, aabb.min.x, aabb.max.x), Clamp(circle.center.y, aabb.min.y, aabb.max.y)};

    // Check if that closest point is within the circle's radius
    float dx = circle.center.x - closest.x;
    float dy = circle.center.y - closest.y;
    return dx * dx + dy * dy <= circle.radius * circle.radius;
}

inline Circle GetHurtBox(state::PlayerState &player) {
    return {player.position.x, player.position.y, player.hurtbox.radius};
}

inline Vector2 resolveCircleAABB(Circle circle, const AABB &wall) {
    Vector2 closest = {Clamp(circle.center.x, wall.min.x, wall.max.x), Clamp(circle.center.y, wall.min.y, wall.max.y)};

    Vector2 delta = Vector2Subtract(circle.center, closest);
    float dist = Vector2Length(delta);

    if (dist >= circle.radius)
        return circle.center;

    if (dist == 0.0f) {
        float overlapLeft = circle.center.x - wall.min.x;
        float overlapRight = wall.max.x - circle.center.x;
        float overlapTop = circle.center.y - wall.min.y;
        float overlapBottom = wall.max.y - circle.center.y;

        float minOverlap = std::min({overlapLeft, overlapRight, overlapTop, overlapBottom});

        if (minOverlap == overlapLeft)
            return {wall.min.x - circle.radius, circle.center.y};
        else if (minOverlap == overlapRight)
            return {wall.max.x + circle.radius, circle.center.y};
        else if (minOverlap == overlapTop)
            return {circle.center.x, wall.min.y - circle.radius};
        else
            return {circle.center.x, wall.max.y + circle.radius};
    }

    Vector2 pushDir = Vector2Scale(delta, 1.0f / dist); // normalize
    return Vector2Add(closest, Vector2Scale(pushDir, circle.radius));
}

inline Vector2 resolveCircleAABBList(Circle circle, const std::vector<AABB> &walls) {
    Vector2 pos = circle.center;
    for (const auto &wall : walls) {
        circle.center = pos;
        pos = resolveCircleAABB(circle, wall);
    }
    return pos;
}
} // namespace Collision
