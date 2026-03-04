#pragma once

#include "../components/hurtbox.hpp"
#include "../state/player_state.hpp"
#include "raylib.h"
namespace Shapes {

struct Circle {
    float x, y, radius;
};

inline bool Overlap(Circle a, Circle b) {
    float dx = a.x - b.x, dy = a.y - b.y;
    float r = a.radius + b.radius;
    return dx * dx + dy * dy <= r * r;
}

inline Vector2 CircleToVector(Circle c) { return Vector2(c.x, c.y); }

inline Circle GetHurtBox(state::PlayerState &player) {
    return {player.position.x, player.position.y, player.hurtbox.radius};
}

} // namespace Shapes
