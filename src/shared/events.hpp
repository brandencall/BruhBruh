#pragma once

#include "../shared/map/grid.hpp"
#include "characters/character_types.hpp"
#include "raylib.h"
#include "state/active_effect.hpp"
#include "state/player_state.hpp"
#include <stdint.h>

namespace event {

struct BulletSpawnEvent {
    uint32_t bulletId;
    uint32_t ownerId;
    Character::CharacterId characterId;
    uint32_t bulletPredSequence;
    Vector2 position;
    Vector2 velocity;
};

struct BulletDestroyedEvent {
    uint32_t bulletId;
    Vector2 position;
    Character::CharacterId characterId;
};

struct PlayerRespawnEvent {
    state::PlayerState player;
};

struct PlayerDamagedEvent {
    uint32_t victimId;
    uint32_t attackerId;
    float currentHealth;
};

struct PlayerDiedEvent {
    state::PlayerState victim;
    state::PlayerState killer;
};

struct PlaceWallEvent {
    Map::Vector2i gridPos;
    float maxHealth;
    state::PlayerState player;
};

struct DestroyWallEvent {
    Map::Vector2i gridPos;
    state::PlayerState player;
};

struct WallPickedUpEvent {
    Map::Vector2i gridPos;
    state::PlayerState player;
};

struct DamageWallEvent {
    Map::Vector2i gridPos;
    float currentHealth;
    uint32_t ownerId;
};

struct PowerUpSpawnEvent {
    uint32_t id;
    state::PickupType pickupType;
    uint8_t typeId;
    Vector2 position;
    float radius;
};

struct PowerUpDespawnEvent {
    uint32_t id;
};

} // namespace event
