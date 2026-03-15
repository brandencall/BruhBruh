#pragma once
#include "../config.hpp"
#include "../shared/characters/character_types.hpp"
#include "../shared/state/player_state.hpp"
#include "map/grid.hpp"
#include <stdint.h>

namespace network {

enum class PacketType : uint8_t {
    Join,
    Disconnect,
    JoinResponse,
    Input,
    State,

    BulletSpawn,
    BulletHit,
    BulletExpired,

    PlayerDied,

    PlaceWall,
    WallDamaged,
    WallDestroyed
};

struct PacketHeader {
    PacketType type;
};

struct JoinPacket {
    PacketHeader header;
};

struct DisconnectPacket {
    PacketHeader header;
    uint32_t playerId;
};

struct JoinResponsePacket {
    PacketHeader header;
    uint32_t playerId;
    Character::CharacterId characterId;
};

struct InputPacket {
    PacketHeader header;
    uint32_t playerId;
    Character::CharacterId characterId;

    float moveX;
    float moveY;
    float aimX;
    float aimY;
    // bitmask (shoot, place_wall, etc.)
    uint8_t buttons;

    uint32_t sequence;
};

struct StatePacket {
    PacketHeader header;
    uint32_t tick; // server tick number
    uint16_t playerCount;
    state::PlayerState players[MAX_PLAYERS];
};

struct BulletSpawnPacket {
    PacketHeader header;
    uint32_t bulletId;
    uint32_t ownerId;
    Character::CharacterId characterId;
    Vector2 position;
    Vector2 velocity;
};

struct BulletHitPacket {
    PacketHeader header;
    uint32_t bulletId;
    uint32_t victimId;
    Vector2 hitPosition;
};

struct BulletExpirePacket {
    PacketHeader header;
    uint32_t bulletId;
};

struct PlayerDiedPacket {
    PacketHeader header;
    uint32_t id;
    Character::CharacterId characterId;
    float respawnTimer;
};

struct PlaceWallPacket {
    PacketHeader header;
    Map::Vector2i gridPos;
    float maxHealth;
    uint32_t ownerId;
};

struct WallDestroyedPacket {
    PacketHeader header;
    Map::Vector2i gridPos;
    uint32_t ownerId;
};

} // namespace network
