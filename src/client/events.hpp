#pragma once

#include "../shared/characters/character_types.hpp"
#include "../shared/events.hpp"
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

} // namespace client
