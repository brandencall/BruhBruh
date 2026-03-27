#include "state_broadcaster.hpp"
#include <cstddef>

namespace network {

void StateBroadcaster::BroadcastState(const GameSimulation &sim) {
    BuildStatePacket(sim);
    size_t sendSize = offsetof(network::StatePacket, players) + m_statePacket.playerCount * sizeof(state::PlayerState);

    m_registry.ForEach(
        [&](network::ClientConnection &client) { m_transport.send(client.peerId, &m_statePacket, sendSize); });
}

void StateBroadcaster::SendFullSnapshot(network::PeerId peer, const GameSimulation &sim) {
    BuildStatePacket(sim);
    size_t sendSize = offsetof(network::StatePacket, players) + m_statePacket.playerCount * sizeof(state::PlayerState);
    m_transport.send(peer, &m_statePacket, sendSize);
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

void StateBroadcaster::BuildStatePacket(const GameSimulation &sim) {
    m_statePacket.header.type = network::PacketType::State;
    m_statePacket.tick = m_tick;
    m_statePacket.playerCount = 0;

    const auto &players = sim.GetPlayers();
    for (int i = 0; i < MAX_PLAYERS; ++i) {
        if (!players[i].active)
            continue;

        uint16_t slot = m_statePacket.playerCount++;
        const state::PlayerState &p = players[i];
        m_statePacket.players[slot].id = p.id;
        m_statePacket.players[slot].characterId = p.characterId;
        m_statePacket.players[slot].position.x = p.position.x;
        m_statePacket.players[slot].position.y = p.position.y;
        m_statePacket.players[slot].health = p.health;
        m_statePacket.players[slot].hurtbox = p.hurtbox;
        m_statePacket.players[slot].respawnTimer = p.respawnTimer;
        m_statePacket.players[slot].active = p.active ? 1 : 0;
    }
}

} // namespace network
