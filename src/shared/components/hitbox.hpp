#pragma once

#include "collision.hpp"

namespace component {
struct Hitbox {
    Collision::Circle circle;
    float damage;
};
} // namespace component
