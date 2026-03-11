#pragma once
#include "../characters/character_types.hpp"
#include "../components/hurtbox.hpp"
#include "raylib.h"
#include <cstdint>
#include <stdint.h>

namespace state {
struct PlayerInput {
    float moveX;
    float moveY;
    float aimX;
    float aimY;
    // bitmask (shoot, place_wall, etc.)
    uint8_t buttons;
};

struct PlayerState {
    uint32_t id = UINT32_MAX;
    Character::CharacterId characterId = Character::CharacterId::None;
    Vector2 position = {0, 0};
    Vector2 velocity = {0, 0};
    float speed = 300.0f;
    float health = 100;
    component::Hurtbox hurtbox;
    PlayerInput currentInput;
    // bitmask (shoot, place_wall, etc.)
    uint8_t lastButtons;
    bool alive = false;
    bool active = false;
};
} // namespace state
