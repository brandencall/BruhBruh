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
             650.0f,                                          // speed
             8.0f,                                            // radius
             10.0f,                                           // damage
             3.0f,                                            // lifetime
             540.0f,                                          // spinSpeed
             .10f,                                            // cooldown
             1.25f,                                           // bulletTexScale
             "assets/sounds/characters/glass_break_tonts.wav" // bulletSoundLocation
         }},
        {CharacterId::Hodge, // CharacterId
         300.0f,             // moveSpeed
         12.0f,              // hurtboxRadius
         100.0f,             // maxHealth
         5,                  // maxWalls
         .50f,               // wallCooldown
         // BulletDef
         {
             650.0f,                                            // speed
             8.0f,                                              // radius
             10.0f,                                             // damage
             3.0f,                                              // lifetime
             360.0f,                                            // spinSpeed
             .10f,                                              // cooldown
             1.25f,                                             // bulletTexScale
             "assets/sounds/characters/meatball_hit_hodges.wav" // bulletSoundLocation
         }},
        {CharacterId::Raff, // CharacterId
         300.0f,            // moveSpeed
         12.0f,             // hurtboxRadius
         100.0f,            // maxHealth
         5,                 // maxWalls
         .50f,              // wallCooldown
         // BulletDef
         {
             650.0f,                                       // speed
             8.0f,                                         // radius
             10.0f,                                        // damage
             3.0f,                                         // lifetime
             360.0f,                                       // spinSpeed
             .10f,                                         // cooldown
             1.25f,                                        // bulletTexScale
             "assets/sounds/characters/steak_hit_raff.wav" // bulletSoundLocation
         }},
        {CharacterId::JJ, // CharacterId
         300.0f,          // moveSpeed
         12.0f,           // hurtboxRadius
         100.0f,          // maxHealth
         5,               // maxWalls
         .50f,            // wallCooldown
         // BulletDef
         {
             650.0f,                                      // speed
             8.0f,                                        // radius
             10.0f,                                       // damage
             3.0f,                                        // lifetime
             0.0f,                                        // spinSpeed
             .10f,                                        // cooldown
             2.50f,                                       // bulletTexScale
             "assets/sounds/characters/needle_hit_jj.wav" // bulletSoundLocation
         }},
    };
    return roster[static_cast<uint8_t>(id)];
}

} // namespace Character
