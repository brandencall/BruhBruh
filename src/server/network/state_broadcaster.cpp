#include "state_broadcaster.hpp"
#include <cstddef>
#include <cstdint>

namespace network {

void StateBroadcaster::BroadcastState(const GameSimulation &sim) {
    network::StatePacket statePacket{};
    BuildStatePacket(sim, statePacket);
    size_t sendSize = offsetof(network::StatePacket, players) + statePacket.playerCount * sizeof(state::PlayerState);

    m_registry.ForEach(
        [&](network::ClientConnection &client) { m_transport.send(client.peerId, &statePacket, sendSize); });
}

void StateBroadcaster::SendCurrentWorldState(network::PeerId peer, const GameSimulation &sim) {
    network::CurrentWorldStatePacket worldStatePacket{};
    BuildCurrentWorldStatePacket(sim, worldStatePacket);
    size_t sendSize =
        offsetof(network::StatePacket, players) + worldStatePacket.playerCount * sizeof(state::PlayerState);

    m_registry.ForEach(
        [&](network::ClientConnection &client) { m_transport.send(client.peerId, &worldStatePacket, sendSize); });
}

void StateBroadcaster::BroadcastAll(const void *data, size_t size) {
    m_registry.ForEach([&](network::ClientConnection &client) { m_transport.send(client.peerId, data, size); });
}

void StateBroadcaster::DrainAndBroadcast(EventBus &eventBus) {
    eventBus.DrainBulletSpawn([&](const event::BulletSpawnEvent &e) {
        network::BulletSpawnPacket pkt{};
        pkt.header.type = network::PacketType::BulletSpawn;
        pkt.bulletId = e.bulletId;
        pkt.ownerId = e.ownerId;
        pkt.characterId = e.characterId;
        pkt.position = e.position;
        pkt.velocity = e.velocity;
        BroadcastAll(&pkt, sizeof(pkt));
    });
    eventBus.DrainBulletDestroyed([&](const event::BulletDestroyedEvent &e) {
        network::BulletDestroyedPacket pkt{};
        pkt.header.type = network::PacketType::BulletDestroyed;
        pkt.bulletId = e.bulletId;
        BroadcastAll(&pkt, sizeof(pkt));
    });
    eventBus.DrainPlayerRespawn([&](const event::PlayerRespawnEvent &e) {
        network::PlayerRespawnedPacket pkt{};
        pkt.header.type = network::PacketType::PlayerRespawned;
        pkt.player = e.player;
        BroadcastAll(&pkt, sizeof(pkt));
    });
    eventBus.DrainPlayerDamaged([&](const event::PlayerDamagedEvent &e) {
        network::PlayerDamagedPacket pkt{};
        pkt.header.type = network::PacketType::PlayerDamaged;
        pkt.id = e.id;
        pkt.currentHealth = e.currentHealth;
        BroadcastAll(&pkt, sizeof(pkt));
    });
    eventBus.DrainPlayerDeath([&](const event::PlayerDiedEvent &e) {
        network::PlayerDiedPacket pkt{};
        pkt.header.type = network::PacketType::PlayerDied;
        pkt.id = e.id;
        pkt.characterId = e.characterId;
        pkt.respawnTimer = e.respawnTimer;
        BroadcastAll(&pkt, sizeof(pkt));
    });
    eventBus.DrainPlaceWall([&](const event::PlaceWallEvent &e) {
        network::PlaceWallPacket pkt{};
        pkt.header.type = network::PacketType::PlaceWall;
        pkt.gridPos = e.gridPos;
        pkt.maxHealth = e.maxHealth;
        pkt.ownerId = e.ownerId;
        BroadcastAll(&pkt, sizeof(pkt));
    });
    eventBus.DrainDamageWall([&](const event::DamageWallEvent &e) {
        network::WallDamagedPacket pkt{};
        pkt.header.type = network::PacketType::WallDamaged;
        pkt.gridPos = e.gridPos;
        pkt.ownerId = e.ownerId;
        pkt.currentHealth = e.currentHealth;
        BroadcastAll(&pkt, sizeof(pkt));
    });
    eventBus.DrainDestroyWall([&](const event::DestroyWallEvent &e) {
        network::WallDestroyedPacket pkt{};
        pkt.header.type = network::PacketType::WallDestroyed;
        pkt.gridPos = e.gridPos;
        pkt.ownerId = e.ownerId;
        BroadcastAll(&pkt, sizeof(pkt));
    });
}

void StateBroadcaster::BuildStatePacket(const GameSimulation &sim, network::StatePacket &statePacket) {
    statePacket.header.type = network::PacketType::State;
    statePacket.tick = m_tick;
    statePacket.playerCount = BuildPlayerState(sim, statePacket.players);
}

void StateBroadcaster::BuildCurrentWorldStatePacket(const GameSimulation &sim,
                                                    network::CurrentWorldStatePacket &worldStatePacket) {

    worldStatePacket.header.type = network::PacketType::CurrentWorldState;
    worldStatePacket.tick = m_tick;
    worldStatePacket.playerCount = BuildPlayerState(sim, worldStatePacket.players);

    auto walls = sim.GetWallManager().GetAllWalls();
    uint16_t i = 0;
    for (auto &[key, wall] : walls) {
        worldStatePacket.walls[i++] = {key, wall};
    }
    worldStatePacket.wallCount = i;
}

uint16_t StateBroadcaster::BuildPlayerState(const GameSimulation &sim, state::PlayerState *players) {
    uint16_t playerCount = 0;
    const auto &currentPlayers = sim.GetPlayers();

    for (int i = 0; i < MAX_PLAYERS; ++i) {
        if (!currentPlayers[i].active)
            continue;

        uint16_t slot = playerCount++;
        players[slot] = currentPlayers[i];
        players[slot].active = currentPlayers[i].active ? 1 : 0;
    }

    return playerCount;
}

} // namespace network
