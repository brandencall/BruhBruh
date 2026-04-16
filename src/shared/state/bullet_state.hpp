#pragma once
#include "../characters/character_types.hpp"
#include "../components/hitbox.hpp"
#include <cstdint>
#include <stdint.h>

namespace state {
struct BulletState {
    uint32_t id;
    uint32_t ownerId;
    Character::CharacterId characterId;
    Vector2 velocity;
    float lifetime;
    float rotation;
    component::Hitbox hitbox;
    bool active;
};
} // namespace state
