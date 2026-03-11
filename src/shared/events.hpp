#pragma once

#include "characters/character_types.hpp"
#include "raylib.h"
#include <stdint.h>

namespace event {

struct BulletSpawnEvent {
    uint32_t bulletId;
    uint32_t ownerId;
    Character::CharacterId characterId;
    Vector2 position;
    Vector2 velocity;
};

struct BulletHitEvent {
    uint32_t bulletId;
    uint32_t victimId;
    Vector2 hitPosition;
};

struct BulletExpireEvent {
    uint32_t bulletId;
};

struct PlayerDiedEvent {
    uint32_t id;
    Character::CharacterId characterId;
    float respawnTimer;
};

} // namespace event
