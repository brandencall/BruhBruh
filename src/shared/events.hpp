#pragma once

#include "../shared/map/grid.hpp"
#include "characters/character_types.hpp"
#include "raylib.h"
#include "state/player_state.hpp"
#include <stdint.h>

namespace event {

struct BulletSpawnEvent {
    uint32_t bulletId;
    uint32_t ownerId;
    Character::CharacterId characterId;
    Vector2 position;
    Vector2 velocity;
};

struct BulletDestroyedEvent {
    uint32_t bulletId;
};

struct PlayerRespawnEvent {
    state::PlayerState player;
};

struct PlayerDamagedEvent {
    uint32_t id;
    float currentHealth;
};

struct PlayerDiedEvent {
    uint32_t id;
    Character::CharacterId characterId;
    float respawnTimer;
};

struct PlaceWallEvent {
    Map::Vector2i gridPos;
    float maxHealth;
    state::PlayerState player;
};

struct DestroyWallEvent {
    Map::Vector2i gridPos;
    uint32_t ownerId;
};

struct DamageWallEvent {
    Map::Vector2i gridPos;
    float currentHealth;
    uint32_t ownerId;
};

} // namespace event
