#pragma once

#include "../game_simulation.hpp"
#include "../server_transport.hpp"
#include "client_registry.hpp"
#include "state_broadcaster.hpp"

namespace network {
class PacketHandler {
  public:
    PacketHandler(network::ServerTransport &transport, ClientRegistry &registry, GameSimulation &simulation,
                  StateBroadcaster &broadcaster)
        : m_transport(transport), m_registry(registry), m_simulation(simulation), m_broadcaster(broadcaster) {}

    void Handle(char *buffer, size_t bytes, network::PeerId from);

  private:
    void OnJoin(network::PeerId from);
    void OnDisconnect(char *buffer, network::PeerId from);
    void OnInput(char *buffer, size_t bytes, network::PeerId from);

    void SendJoinResponse(network::PeerId to, uint32_t playerId, Character::CharacterId charId);

  private:
    network::ServerTransport &m_transport;
    ClientRegistry &m_registry;
    GameSimulation &m_simulation;
    StateBroadcaster &m_broadcaster;
};
} // namespace network
