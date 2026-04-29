
#pragma once
#include "../characters/character_types.hpp"
#include <cstdint>
#include <stdint.h>

namespace state {
struct LobbySlotState {
    uint32_t id = UINT32_MAX;
    Character::CharacterId characterId = Character::CharacterId::None;
    char name[64] = {};
    bool ready = false;
    bool occupied = false;
};
} // namespace state
