#pragma once
#include "../../network/packets/gameplay_packets.hpp"
#include "../../shared/characters/character_types.hpp"
#include "../../shared/network/ITransport.hpp"
#include "../../shared/state/player_state.hpp"
#include "../game_simulation.hpp"
#include "../server_lobby.hpp"
#include "client_registry.hpp"
#include <cstdint>

namespace network {
class StateBroadcaster {

  public:
    StateBroadcaster(ClientRegistry &registry, int &tick) : m_registry(registry), m_tick(tick) {}

    void SetTransport(network::ITransport &transport);
    void AssertTransport() const;

    void BroadcastStartGame(float countdown);
    void BroadcastCharacterSelected(uint32_t playerId, Character::CharacterId characterId);
    void BroadcastGameBegin(float countdown, const GameSimulation &sim);
    void BroadcastGameEnd(float countdown, const GameSimulation &sim);
    void BroadcastPlayerJoined(const char *name, ClientConnection *client);
    void BroadcastPlayerDisconnect(uint32_t playerId);
    void BroadcastState(const GameSimulation &sim);
    void BroadcastLobbyState(const ServerLobby &lobby);
    void BroadcastSwitchToLobby();
    void BroadcastCurrentWorldState(network::PeerId peer, const GameSimulation &sim);
    void DrainAndBroadcast(EventBus &eventBus);
    void BroadcastAll(const void *data, size_t size);

  private:
    void BuildStatePacket(const GameSimulation &sim, network::StatePacket &statePacket);
    void BuildCurrentWorldStatePacket(const GameSimulation &sim, network::CurrentWorldStatePacket &worldStatePacket);
    // Creates the player state and returns the count of the current players in the player state
    uint16_t BuildPlayerState(const GameSimulation &sim, state::PlayerState *players);
    uint16_t BuildRankedPlayers(const GameSimulation &sim, state::RankedPlayer *players);

  private:
    network::ITransport *m_transport = nullptr;
    ClientRegistry &m_registry;
    int &m_tick;
};
} // namespace network
