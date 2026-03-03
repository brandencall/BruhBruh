#pragma once
#include "raylib.h"
#include <cstdint>
#include <stdint.h>

namespace state {
struct PlayerInput {
    float moveX;
    float moveY;
    // bitmask (shoot, place_wall, etc.)
    uint8_t buttons;
};

struct PlayerState {
    uint32_t id = UINT32_MAX;
    Vector2 position = {0, 0};
    Vector2 velocity = {0, 0};
    float speed = 300.0f;
    float health = 100;
    bool active = false;

    PlayerInput currentInput;
};
} // namespace state
