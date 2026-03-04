#pragma once
#include "../config.hpp"
#include "../shared/state/bullet_state.hpp"
#include "../shared/state/player_state.hpp"
#include <stdint.h>

namespace network {

enum class PacketType : uint8_t { Join, Disconnect, JoinResponse, Input, State };

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
};

struct InputPacket {
    PacketHeader header;
    uint32_t playerId;

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
    uint16_t bulletCount;
    state::BulletState bullets[MAX_BULLETS];
};

} // namespace network
