#pragma once
#include "raylib.h"
#include <stdint.h>

struct PlayerInput {
    float moveX;
    float moveY;
    // bitmask (shoot, place_wall, etc.)
    uint8_t buttons;
};

struct PlayerState {
    uint32_t id = -1;
    Vector2 position = {0, 0};
    Vector2 velocity = {0, 0};
    float speed = 300.0f;
    float health = 100;
    bool active = false;

    PlayerInput currentInput;
};
