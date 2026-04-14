#pragma once
#include <stdint.h>

namespace network {

enum class PacketType : uint8_t {
    // -- Lobby ---------------------
    LobbyBegin = 0,
    JoinLobby,
    JoinResponse,
    PlayerJoined,
    PlayerReady,
    LobbyState,
    Disconnect,
    StartGame,
    CharacterSelected,
    LobbyEnd,

    // -- Gameplay -----------------
    GameBegin,
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
    WallDestroyed,
    GameEnd
};

inline bool IsLobbyPacket(PacketType t) { return t > PacketType::LobbyBegin && t < PacketType::LobbyEnd; }

inline bool IsGamePacket(PacketType t) { return t > PacketType::GameBegin && t < PacketType::GameEnd; }

struct PacketHeader {
    PacketType type;
};

} // namespace network
