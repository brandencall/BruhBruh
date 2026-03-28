#pragma once

#include "../game_simulation.hpp"
#include "../network/packet.hpp"
#include "../server/server_transport.hpp"
#include "client_registry.hpp"
#include "state/player_state.hpp"
#include <cstdint>

namespace network {
class StateBroadcaster {

  public:
    StateBroadcaster(network::ServerTransport &transport, ClientRegistry &registry, int &tick)
        : m_transport(transport), m_registry(registry), m_tick(tick) {}

    void BroadcastState(const GameSimulation &sim);
    void SendCurrentWorldState(network::PeerId peer, const GameSimulation &sim);
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
