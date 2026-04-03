#pragma once
#include "../../config.hpp"
#include "../characters/character_types.hpp"
#include "../components/hurtbox.hpp"
#include "raylib.h"
#include <cstdint>
#include <stdint.h>

namespace state {

struct PlayerScore {
    uint8_t kills = 0;
    uint8_t deaths = 0;
};

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
    char name[MAX_PLAYER_NAME_LEN] = {};
    Vector2 position = {0, 0};
    Vector2 velocity = {0, 0};
    float speed = 300.0f;
    float health = 100;
    component::Hurtbox hurtbox;
    PlayerScore score;
    PlayerInput currentInput;
    // bitmask (shoot, place_wall, etc.)
    uint8_t lastButtons;
    float respawnTimer = 0.0f;
    uint32_t currentAvaliableWalls = 5;
    bool active = false;
};
} // namespace state
