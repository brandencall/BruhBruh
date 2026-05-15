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
    HostDisconnected,
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
    WallPickedUp,

    GameEnd,
    SwitchToLobby
};

inline bool IsLobbyPacket(PacketType t) { return t > PacketType::LobbyBegin && t < PacketType::LobbyEnd; }

inline bool IsGamePacket(PacketType t) { return t > PacketType::GameBegin && t < PacketType::GameEnd; }

inline bool IsReliablePacket(PacketType t) {
    return !IsGamePacket(t) // all lobby packets are reliable
           || t == PacketType::BulletSpawn || t == PacketType::BulletDestroyed || t == PacketType::PlayerRespawned ||
           t == PacketType::PlayerDamaged || t == PacketType::PlayerDied || t == PacketType::PlaceWall ||
           t == PacketType::WallDamaged || t == PacketType::WallDestroyed || t == PacketType::GameEnd ||
           t == PacketType::SwitchToLobby;
}

struct PacketHeader {
    PacketType type;
};

} // namespace network
