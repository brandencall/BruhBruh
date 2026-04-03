#pragma once
#include "../config.hpp"
#include "../shared/characters/character_types.hpp"
#include "../shared/map/dynamic_walls/dynamic_wall.hpp"
#include "../shared/map/grid.hpp"
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
    CurrentWorldState,

    BulletSpawn,
    BulletDestroyed,

    PlayerRespawned,
    PlayerDamaged,
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
    Vector2 position;
    Vector2 velocity;
};

struct BulletDestroyedPacket {
    PacketHeader header;
    uint32_t bulletId;
};

struct PlayerRespawnedPacket {
    PacketHeader header;
    state::PlayerState player;
};

struct PlayerDamagedPacket {
    PacketHeader header;
    uint32_t id;
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

} // namespace network
