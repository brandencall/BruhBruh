#pragma once

#include "raylib.h"
#include <stdint.h>

namespace event {

struct BulletSpawnEvent {
    uint32_t bulletId;
    uint32_t ownerId;
    Vector2 position;
    Vector2 velocity;
    float lifetime;
};

struct BulletHitEvent {
    uint32_t bulletId;
    uint32_t victimId;
    Vector2 hitPosition;
};

struct BulletExpireEvent {
    uint32_t bulletId;
};

} // namespace event
