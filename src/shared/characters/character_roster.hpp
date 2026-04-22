#pragma once
#include "character_types.hpp"

namespace Character {

inline const CharacterDef &GetCharacterDef(CharacterId id) {
    static const CharacterDef roster[] = {
        {},
        {CharacterId::Tonts, 300.0f, 12.0f, 100.0f, .50f, {500.0f, 10.0f, 10.0f, 3.0f, 360.0f, .20f}},
        {CharacterId::Hodge, 300.0f, 12.0f, 100.0f, .50f, {500.0f, 10.0f, 10.0f, 3.0f, 360.0f, .20f}},
        {CharacterId::Raff, 300.0f, 12.0f, 100.0f, .50f, {500.0f, 10.0f, 10.0f, 3.0f, 360.0f, .20f}},
        {CharacterId::JJ, 300.0f, 12.0f, 100.0f, .50f, {500.0f, 10.0f, 10.0f, 3.0f, 0.0f, .20f}},
    };
    return roster[static_cast<uint8_t>(id)];
}

} // namespace Character
