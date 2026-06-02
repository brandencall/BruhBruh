#pragma once
#include "../../config.hpp"
#include "../../shared/characters/character_types.hpp"
#include "../../shared/map/dynamic_walls/dynamic_wall.hpp"
#include "../../shared/map/grid.hpp"
#include "../../shared/state/player_state.hpp"
#include "packet_header.hpp"
#include <cstdint>

namespace network {

struct GameBeginPacket {
    PacketHeader header;
    float countdown;
    float gameTime;
    uint16_t playerCount;
    state::PlayerState players[MAX_PLAYERS];
};

struct GameEndPacket {
    PacketHeader header;
    float countdown;
    uint16_t playerCount;
    state::RankedPlayer rankedPlayers[MAX_PLAYERS];
    // Send the player rankings
};

struct InputPacket {
    PacketHeader header;
    uint32_t playerId;
    Character::CharacterId characterId;

    float moveX;
    float moveY;
    float aimX;
    float aimY;
    float facingAngle;
    // bitmask (shoot, place_wall, etc.)
    uint8_t buttons;

    uint32_t sequence;
    uint32_t predBulletSequence = UINT32_MAX;
};

struct StatePacket {
    PacketHeader header;
    uint32_t tick; // server tick number
    uint32_t currentGameTime;
    uint16_t playerCount;
    state::PlayerState players[MAX_PLAYERS];
};

struct WallEntry {
    Map::Vector2i position;
    Map::DynamicWall wall;
};

struct CurrentWorldStatePacket {
    PacketHeader header;
    uint32_t tick;
    uint16_t wallCount;
    WallEntry walls[MAX_WALLS];
    uint16_t playerCount;
    state::PlayerState players[MAX_PLAYERS];
};

struct BulletSpawnPacket {
    PacketHeader header;
    uint32_t bulletId;
    uint32_t ownerId;
    Character::CharacterId characterId;
    uint32_t bulletPredSequence;
    Vector2 position;
    Vector2 velocity;
};

struct BulletDestroyedPacket {
    PacketHeader header;
    uint32_t bulletId;
    Vector2 position;
    Character::CharacterId characterId;
};

struct PlayerRespawnedPacket {
    PacketHeader header;
    state::PlayerState player;
};

struct PlayerDamagedPacket {
    PacketHeader header;
    uint32_t vitimId;
    uint32_t attackerId;
    float currentHealth;
};

struct PlayerDiedPacket {
    PacketHeader header;
    // Change this to victim
    state::PlayerState victim;
    state::PlayerState killer;
};

struct PlaceWallPacket {
    PacketHeader header;
    Map::Vector2i gridPos;
    float maxHealth;
    state::PlayerState player;
};

struct WallDamagedPacket {
    PacketHeader header;
    Map::Vector2i gridPos;
    uint32_t ownerId;
    float currentHealth;
};

struct WallDestroyedPacket {
    PacketHeader header;
    Map::Vector2i gridPos;
    state::PlayerState player;
};

struct WallPickedUpPacket {
    PacketHeader header;
    Map::Vector2i gridPos;
    state::PlayerState player;
};

struct WallInputDeniedPacket {
    PacketHeader header;
    uint32_t playerId;
};

struct PowerUpSpawnPacket {
    PacketHeader header;
    uint32_t id;
    state::PickupType pickupType;
    uint8_t typeId;
    Vector2 position;
    float radius;
};

struct PowerUpDespawnPacket {
    PacketHeader header;
    uint32_t id;
};

} // namespace network
