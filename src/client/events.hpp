#pragma once

#include "../shared/characters/character_types.hpp"
#include "../shared/events.hpp"
#include "raylib.h"
#include <stdint.h>

namespace client {

struct PlayerDiedEvent {
    event::PlayerDiedEvent data;
    state::PlayerState localPlayer;
};

struct HitEvent {
    uint32_t attackerId;
    uint32_t victimId;
    Character::CharacterId attackerCharacter;
    uint32_t localPlayerId;
};

struct WallPlacedEvent {
    uint32_t wallPlacerId;
    Map::Vector2i gridPos;
    uint32_t localPlayerId;
    Vector2 localPlayerPosition;
};

struct WallPickedUpEvent {
    Map::Vector2i gridPos;
    uint32_t localPlayerId;
    Vector2 localPlayerPosition;
};

} // namespace client
