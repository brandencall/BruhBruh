#pragma once
#include "../../shared/network/ITransport.hpp"
#include "../game_simulation.hpp"
#include "../server_lobby.hpp"
#include "../server_phase.hpp"
#include "client_registry.hpp"
#include "state_broadcaster.hpp"

namespace network {
class PacketHandler {
  public:
    PacketHandler(ClientRegistry &registry, GameSimulation &simulation, StateBroadcaster &broadcaster,
                  ServerPhase &phase, ServerLobby &lobby)
        : m_registry(registry), m_simulation(simulation), m_broadcaster(broadcaster), m_phase(phase), m_lobby(lobby) {}

    void Handle(char *buffer, size_t bytes, network::PeerId from);
    void SetTransport(network::ITransport &transport);

  private:
    void OnJoinLobby(char *buffer, size_t size, network::PeerId from);
    void OnPlayerReady(char *buffer, size_t size, network::PeerId from);
    void OnCharacterSelected(char *buffer, size_t size, network::PeerId from);
    void OnDisconnect(char *buffer, network::PeerId from);
    void OnInput(char *buffer, size_t bytes, network::PeerId from);

    void SendJoinResponse(network::PeerId to, uint32_t playerId);

  private:
    network::ITransport *m_transport;
    ClientRegistry &m_registry;
    GameSimulation &m_simulation;
    StateBroadcaster &m_broadcaster;
    ServerPhase &m_phase;
    ServerLobby &m_lobby;
};
} // namespace network
