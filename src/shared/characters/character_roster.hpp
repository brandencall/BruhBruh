#pragma once
#include "character_types.hpp"

namespace Character {

inline const CharacterDef &GetCharacterDef(CharacterId id) {
    static const CharacterDef roster[] = {
        {},
        {CharacterId::Tonts, 300.0f, 12.0f, 100.0f, 1.0f, {500.0f, 10.0f, 10.0f, 3.0f, 360.0f}},
        {CharacterId::Hodge, 300.0f, 12.0f, 100.0f, 1.0f, {500.0f, 10.0f, 10.0f, 3.0f, 360.0f}},
        {CharacterId::Raff, 300.0f, 12.0f, 100.0f, 1.0f, {500.0f, 10.0f, 10.0f, 3.0f, 360.0f}},
        {CharacterId::JJ, 300.0f, 12.0f, 100.0f, 1.0f, {500.0f, 10.0f, 10.0f, 3.0f, 0.0f}},
    };
    return roster[static_cast<uint8_t>(id)];
}

} // namespace Character
