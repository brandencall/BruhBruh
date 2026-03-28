#include "packet_handler.hpp"
#include "../network/packet.hpp"
#include "characters/character_types.hpp"
#include "packet_handler.hpp"
#include <iostream>

namespace network {

void PacketHandler::Handle(char *buffer, size_t bytes, network::PeerId from) {
    auto *header = reinterpret_cast<network::PacketHeader *>(buffer);

    switch (header->type) {
    case network::PacketType::Join:
        OnJoin(from);
        break;
    case network::PacketType::Input:
        OnInput(buffer, bytes, from);
        break;
    case network::PacketType::Disconnect:
        OnDisconnect(buffer, from);
        break;
    default:
        break;
    }
}

void PacketHandler::OnJoin(network::PeerId from) {
    // Peer already connected — resend their join response (handles duplicate join packets)
    auto *existing = m_registry.FindByPeer(from);
    if (existing) {
        SendJoinResponse(from, existing->playerId, existing->characterId);
        return;
    }

    // TODO: character will come from client connection when joinning from loby
    auto *client = m_registry.AddClient(from, Character::CharacterId::Tonts);
    if (!client) {
        std::cout << "Server full\n";
        return;
    }

    m_simulation.CreatePlayer(client->playerId, client->characterId);
    SendJoinResponse(from, client->playerId, client->characterId);
    m_broadcaster.SendCurrentWorldState(from, m_simulation);
    // m_broadcaster.SendFullSnapshot(from, m_simulation);
}

void PacketHandler::OnDisconnect(char *buffer, network::PeerId from) {
    auto *client = m_registry.FindByPeer(from);
    if (!client)
        return;

    m_simulation.RemovePlayer(client->playerId);
    m_registry.RemoveClient(from);
}

void PacketHandler::OnInput(char *buffer, size_t size, network::PeerId from) {
    if (size < sizeof(network::InputPacket))
        return;

    auto *packet = reinterpret_cast<network::InputPacket *>(buffer);
    auto *client = m_registry.FindByPeer(from);
    if (!client)
        return;

    // Reject spoofed player IDs
    if (packet->playerId != client->playerId)
        return;

    m_simulation.ApplyInput(client->playerId, packet->characterId,
                            {
                                .moveX = packet->moveX,
                                .moveY = packet->moveY,
                                .aimX = packet->aimX,
                                .aimY = packet->aimY,
                                .buttons = packet->buttons,
                            });
}

void PacketHandler::SendJoinResponse(network::PeerId to, uint32_t playerId, Character::CharacterId charId) {
    network::JoinResponsePacket response{};
    response.header.type = network::PacketType::JoinResponse;
    response.playerId = playerId;
    response.characterId = charId;
    m_transport.send(to, &response, sizeof(response));
}
} // namespace network
