#pragma once

#include "../game_simulation.hpp"
#include "../network/packets/gameplay_packets.hpp"
#include "../server/server_transport.hpp"
#include "../server_lobby.hpp"
#include "characters/character_types.hpp"
#include "client_registry.hpp"
#include "network/ITransport.hpp"
#include "state/player_state.hpp"
#include <cstdint>

namespace network {
class StateBroadcaster {

  public:
    StateBroadcaster(network::ServerTransport &transport, ClientRegistry &registry, int &tick)
        : m_transport(transport), m_registry(registry), m_tick(tick) {}

    void BroadcastStartGame(float countdown);
    void BroadcastCharacterSelected(uint32_t playerId, Character::CharacterId characterId);
    void BroadcastGameBegin(float countdown, const GameSimulation &sim);
    void BroadcastPlayerJoined(const char *name, ClientConnection *client);
    void BroadcastPlayerDisconnect(uint32_t playerId);
    void BroadcastState(const GameSimulation &sim);
    void BroadcastLobbyState(const ServerLobby &lobby);
    void BroadcastCurrentWorldState(network::PeerId peer, const GameSimulation &sim);
    void DrainAndBroadcast(EventBus &eventBus);
    void BroadcastAll(const void *data, size_t size);

  private:
    void BuildStatePacket(const GameSimulation &sim, network::StatePacket &statePacket);
    void BuildCurrentWorldStatePacket(const GameSimulation &sim, network::CurrentWorldStatePacket &worldStatePacket);
    // Creates the player state and returns the count of the current players in the player state
    uint16_t BuildPlayerState(const GameSimulation &sim, state::PlayerState *players);

  private:
    network::ServerTransport &m_transport;
    ClientRegistry &m_registry;
    int &m_tick;
};
} // namespace network
