#pragma once
#include "../../config.hpp"
#include "../characters/character_types.hpp"
#include "../components/hurtbox.hpp"
#include "raylib.h"
#include <cstdint>
#include <stdint.h>

namespace state {

enum class State { Idle, Running, Dead };

struct PlayerScore {
    uint8_t kills = 0;
    uint8_t deaths = 0;
};

struct PlayerInput {
    float moveX;
    float moveY;
    float aimX;
    float aimY;
    float angle = 0.0;
    // bitmask (shoot, place_wall, etc.)
    uint8_t buttons;
    uint32_t sequence;
};

struct PlayerState {
    uint32_t id = UINT32_MAX;
    Character::CharacterId characterId = Character::CharacterId::None;
    State state = State::Idle;
    char name[MAX_PLAYER_NAME_LEN] = {};
    Vector2 position = {0, 0};
    Vector2 velocity = {0, 0};
    float health = 100;
    component::Hurtbox hurtbox;
    PlayerScore score;
    PlayerInput currentInput;

    // bitmask (shoot, place_wall, etc.)
    uint8_t lastButtons;
    float respawnTimer = 0.0f;
    float invincibilityTimer = 0.0f;
    // Could define this in the character definition
    uint32_t currentAvaliableWalls = 5;
    float shootTimer = 0.0f;
    float wallTimer = 0.0f;
    bool active = false;
};

struct RankedPlayer {
    uint32_t id = UINT32_MAX;
    char name[MAX_PLAYER_NAME_LEN] = {};
    PlayerScore score;
};

} // namespace state
