#pragma once
#include "../../config.hpp"
#include "../../shared/characters/character_types.hpp"
#include "../../shared/state/lobby_slot_state.hpp"
#include "packet_header.hpp"

namespace network {

// TODO: Add LobbyBeginPacket
// TODO: Add LobbyEndPacket

struct JoinLobbyPacket {
    PacketHeader header;
    // Maybe have name here on join
};

struct JoinResponsePacket {
    PacketHeader header;
    uint32_t playerId;
    Character::CharacterId characterId;
    char name[32];
};

struct PlayerJoinedPacket {
    PacketHeader header;
    uint32_t playerId;
    Character::CharacterId characterId;
    char name[32];
};

struct PlayerReadyPacket {
    PacketHeader header;
    uint32_t playerId;
    Character::CharacterId characterId;
};

struct LobbyStatePacket {
    PacketHeader header;
    // Need to send the clients playerId
    state::LobbySlotState lobby[MAX_PLAYERS];
};

struct DisconnectPacket {
    PacketHeader header;
    uint32_t playerId;
};

struct StartGamePacket {
    PacketHeader header;
    // Maybe add the lobby info
};

} // namespace network
