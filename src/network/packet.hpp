#pragma once
#include <stdint.h>

namespace network {

enum class PacketType : uint8_t { Join, JoinResponse, Input, State };

struct PacketHeader {
    PacketType type;
};

struct JoinPacket {
    PacketHeader header;
};

struct JoinResponsePacket {
    PacketHeader header;
    uint32_t playerId;
};

struct InputPacket {
    PacketHeader header;
    uint32_t playerId;
};

} // namespace network
