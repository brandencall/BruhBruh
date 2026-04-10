
#pragma once
#include "../characters/character_types.hpp"
#include <stdint.h>

namespace state {
struct LobbySlotState {
    uint32_t id;
    Character::CharacterId characterId = Character::CharacterId::None;
    char name[64] = {};
    bool ready = false;
    bool occupied = false;
};
} // namespace state
