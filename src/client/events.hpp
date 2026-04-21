#pragma once

#include "../shared/characters/character_types.hpp"
#include "raylib.h"
#include <stdint.h>

namespace client {

struct PlayerDiedEvent {
    Vector2 victimPosition;
    Character::CharacterId victimCharacter;
};

struct HitEvent {
    uint32_t attackerId;
    uint32_t victimId;
    Character::CharacterId attackerCharacter;
};

} // namespace client
