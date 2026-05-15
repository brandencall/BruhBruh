#pragma once

#include "../../components/collision.hpp"
#include "../grid.hpp"
#include <cstdint>

namespace Map {

struct DynamicWall {
    Vector2i gridPos;
    float health;
    float maxHealth;
    uint32_t ownerId;
    Character::CharacterId ownerCharacter;
    Collision::AABB collider;
    float spawnTime;
    bool active = false;
};

} // namespace Map
