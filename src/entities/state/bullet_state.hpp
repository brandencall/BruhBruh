#pragma once
#include "raylib.h"
#include <cstdint>
#include <stdint.h>

namespace state {
struct BulletState {
    uint32_t id;
    uint32_t ownerId;
    Vector2 position;
    Vector2 velocity;
    bool active;
    float lifetime; // despawn after N seconds
};
} // namespace state
