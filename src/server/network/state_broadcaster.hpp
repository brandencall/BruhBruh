#pragma once

#include "../game_simulation.hpp"
#include "../network/packet.hpp"
#include "../server/server_transport.hpp"
#include "client_registry.hpp"

namespace network {
class StateBroadcaster {

  public:
    StateBroadcaster(network::ServerTransport &transport, ClientRegistry &registry, int &tick)
        : m_transport(transport), m_registry(registry), m_tick(tick) {}

    void BroadcastState(const GameSimulation &sim);
    void SendFullSnapshot(network::PeerId peer, const GameSimulation &sim);
    void DrainAndBroadcast(EventBus &eventBus);
    void BroadcastAll(const void *data, size_t size);

  private:
    void BuildStatePacket(const GameSimulation &sim);

  private:
    network::ServerTransport &m_transport;
    ClientRegistry &m_registry;
    network::StatePacket m_statePacket{};
    int &m_tick;
};
} // namespace network
