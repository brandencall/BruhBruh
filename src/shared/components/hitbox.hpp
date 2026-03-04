#pragma once

#include "shapes.hpp"

namespace component {
struct Hitbox {
    Shapes::Circle circle;
    float damage;
};
} // namespace component
