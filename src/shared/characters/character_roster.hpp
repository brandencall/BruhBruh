#pragma once
#include "character_types.hpp"

namespace Character {

inline const CharacterDef &GetCharacterDef(CharacterId id) {
    static const CharacterDef roster[] = {
        {},
        {CharacterId::Tonts, // CharacterId
         300.0f,             // moveSpeed
         12.0f,              // hurtboxRadius
         100.0f,             // maxHealth
         5,                  // maxWalls
         .50f,               // wallCooldown
         // BulletDef
         {
             650.0f, // speed
             8.0f,   // radius
             10.0f,  // damage
             3.0f,   // lifetime
             540.0f, // spinSpeed
             .20f,   // cooldown
             1.25f,  // bulletTexScale
         }},
        {CharacterId::Hodge, // CharacterId
         300.0f,             // moveSpeed
         12.0f,              // hurtboxRadius
         100.0f,             // maxHealth
         5,                  // maxWalls
         .50f,               // wallCooldown
         // BulletDef
         {
             650.0f, // speed
             8.0f,   // radius
             10.0f,  // damage
             3.0f,   // lifetime
             360.0f, // spinSpeed
             .20f,   // cooldown
             1.25f,  // bulletTexScale
         }},
        {CharacterId::Raff, // CharacterId
         300.0f,            // moveSpeed
         12.0f,             // hurtboxRadius
         100.0f,            // maxHealth
         5,                 // maxWalls
         .50f,              // wallCooldown
         // BulletDef
         {
             650.0f, // speed
             8.0f,   // radius
             10.0f,  // damage
             3.0f,   // lifetime
             360.0f, // spinSpeed
             .20f,   // cooldown
             1.25f,  // bulletTexScale
         }},
        {CharacterId::JJ, // CharacterId
         300.0f,          // moveSpeed
         12.0f,           // hurtboxRadius
         100.0f,          // maxHealth
         5,               // maxWalls
         .50f,            // wallCooldown
         // BulletDef
         {
             650.0f, // speed
             8.0f,   // radius
             10.0f,  // damage
             3.0f,   // lifetime
             0.0f,   // spinSpeed
             .20f,   // cooldown
             2.50f,  // bulletTexScale
         }},
    };
    return roster[static_cast<uint8_t>(id)];
}

} // namespace Character
