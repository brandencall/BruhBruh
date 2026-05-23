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

struct BulletDestroyedEvent {
    uint32_t bulletId;
    Vector2 position;
    Character::CharacterId characterId;
    Vector2 localPlayerPosition;
};

struct HitEvent {
    uint32_t attackerId;
    uint32_t victimId;
    Character::CharacterId attackerCharacter;
    uint32_t localPlayerId;
    Vector2 victimPosition;
    Vector2 localPlayerPosition;
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

struct GameStartingEvent {
    int prevCountdown;
    int countdown;
    int max;
};

} // namespace client
