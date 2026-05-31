#pragma once
#include "hurtbox.hpp"
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

inline bool Overlap(const Circle &a, const Circle &b) {
    float dx = a.center.x - b.center.x, dy = a.center.y - b.center.y;
    float r = a.radius + b.radius;
    return dx * dx + dy * dy <= r * r;
}

inline bool Overlap(const AABB &a, const AABB &b) {
    if (a.max.x <= b.min.x || a.min.x >= b.max.x)
        return false;
    if (a.max.y <= b.min.y || a.min.y >= b.max.y)
        return false;
    return true;
}

inline bool Overlap(const Circle &circle, const AABB &aabb) {
    Vector2 closest = {Clamp(circle.center.x, aabb.min.x, aabb.max.x), Clamp(circle.center.y, aabb.min.y, aabb.max.y)};

    // Check if that closest point is within the circle's radius
    float dx = circle.center.x - closest.x;
    float dy = circle.center.y - closest.y;
    return dx * dx + dy * dy <= circle.radius * circle.radius;
}

inline Circle HurtboxToCircle(Vector2 position, component::Hurtbox hurtbox) {
    return Circle{.center = {position.x + hurtbox.offsetX, position.y + hurtbox.offsetY}, .radius = hurtbox.radius};
}

inline Vector2 resolveCircleAABB(Circle circle, const AABB &wall) {
    Vector2 closest = {Clamp(circle.center.x, wall.min.x, wall.max.x), Clamp(circle.center.y, wall.min.y, wall.max.y)};

    Vector2 delta = Vector2Subtract(circle.center, closest);
    float dist = Vector2Length(delta);

    if (dist >= circle.radius)
        return circle.center;

    if (dist < 0.001f) {
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

inline Vector2 resolveCircleAABBList(Circle circle, const std::vector<AABB> &listA, const std::vector<AABB> &listB) {
    Vector2 pos = circle.center;
    for (const auto &wall : listA) {
        circle.center = pos;
        pos = resolveCircleAABB(circle, wall);
    }
    for (const auto &wall : listB) {
        circle.center = pos;
        pos = resolveCircleAABB(circle, wall);
    }
    return pos;
}

// Returns true if the line segment from segStart to segEnd passes within
// 'radius' of circleCenter. Also outputs the parameter t [0,1] of closest approach
// so you can find the exact hit position.
inline bool SweptCircleVsCircle(Vector2 segStart, Vector2 segEnd, Vector2 circleCenter, float radius, float &outT) {
    Vector2 d = Vector2Subtract(segEnd, segStart);       // movement vector
    Vector2 f = Vector2Subtract(segStart, circleCenter); // start relative to circle

    float a = Vector2DotProduct(d, d);
    float b = 2.0f * Vector2DotProduct(f, d);
    float c = Vector2DotProduct(f, f) - (radius * radius);

    float discriminant = b * b - 4.0f * a * c;
    if (discriminant < 0.0f)
        return false; // no intersection

    discriminant = sqrtf(discriminant);
    float t0 = (-b - discriminant) / (2.0f * a);
    float t1 = (-b + discriminant) / (2.0f * a);

    // We want the earliest hit that's within this tick's movement [0, 1]
    if (t0 >= 0.0f && t0 <= 1.0f) {
        outT = t0;
        return true;
    }
    if (t1 >= 0.0f && t1 <= 1.0f) {
        outT = t1;
        return true;
    }
    return false;
}

// Swept circle vs AABB — expand the AABB by bullet radius and do
// a ray vs expanded box test
inline bool SweptCircleVsAABB(Vector2 segStart, Vector2 segEnd, const AABB &aabb, float bulletRadius, float &outT) {
    // Expand AABB by bullet radius (Minkowski sum)
    AABB expanded = {{aabb.min.x - bulletRadius, aabb.min.y - bulletRadius},
                     {aabb.max.x + bulletRadius, aabb.max.y + bulletRadius}};

    // Ray vs AABB slab test
    Vector2 d = Vector2Subtract(segEnd, segStart);
    float tMin = 0.0f, tMax = 1.0f;

    auto testAxis = [&](float start, float dir, float min, float max) -> bool {
        if (fabsf(dir) < 1e-6f)
            return start >= min && start <= max; // parallel — check if inside
        float t0 = (min - start) / dir;
        float t1 = (max - start) / dir;
        if (t0 > t1)
            std::swap(t0, t1);
        tMin = std::max(tMin, t0);
        tMax = std::min(tMax, t1);
        return tMin <= tMax;
    };

    if (!testAxis(segStart.x, d.x, expanded.min.x, expanded.max.x))
        return false;
    if (!testAxis(segStart.y, d.y, expanded.min.y, expanded.max.y))
        return false;

    outT = tMin;
    return tMin <= tMax;
}

} // namespace Collision
