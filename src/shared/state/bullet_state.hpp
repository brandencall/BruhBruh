#pragma once
#include "../components/hitbox.hpp"
#include "raylib.h"
#include <cstdint>
#include <stdint.h>

namespace state {
struct BulletState {
    uint32_t id;
    uint32_t ownerId;
    Vector2 velocity;
    float lifetime;
    component::Hitbox hitbox;
    bool active;
};
} // namespace state
