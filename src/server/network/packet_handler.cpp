#include "packet_handler.hpp"
#include "../network/packets/gameplay_packets.hpp"
#include "../network/packets/lobby_packets.hpp"
#include "characters/character_types.hpp"
#include "packet_handler.hpp"
#include <iostream>
#include <string>

namespace network {

void PacketHandler::Handle(char *buffer, size_t bytes, network::PeerId from) {
    auto *header = reinterpret_cast<network::PacketHeader *>(buffer);

    switch (header->type) {
    case network::PacketType::JoinLobby:
        OnJoinLobby(buffer, bytes, from);
        break;
    case network::PacketType::PlayerReady:
        OnPlayerReady(buffer, bytes, from);
        break;
    case network::PacketType::CharacterSelected:
        OnCharacterSelected(buffer, bytes, from);
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

void PacketHandler::OnJoinLobby(char *buffer, size_t size, network::PeerId from) {
    // Only allow joins during lobby phase
    auto *existing = m_registry.FindByPeer(from);
    if (m_phase != ServerPhase::LOBBY) {
        SendJoinResponse(from, existing->playerId, existing->characterId);
        return;
    }

    // Duplicate join — already in lobby
    if (existing) {
        SendJoinResponse(from, existing->playerId, existing->characterId);
        return;
    }

    auto *pkt = reinterpret_cast<network::JoinLobbyPacket *>(buffer);
    auto *client = m_registry.AddClient(from, Character::CharacterId::None);
    if (!client) {
        // TODO: Send a lobby full packet so that the client knows that it is getting denied
        return;
    }

    int slot = m_lobby.AddPlayer(from, client->playerId);
    SendJoinResponse(from, client->playerId, client->characterId);

    // m_broadcaster.BroadcastPlayerJoined(pkt->name, client);
    std::string name = "Player[" + std::to_string(slot) + "]";
    m_broadcaster.BroadcastPlayerJoined(name.c_str(), client);
}

void PacketHandler::OnPlayerReady(char *buffer, size_t size, network::PeerId from) {
    auto *pkt = reinterpret_cast<network::PlayerReadyPacket *>(buffer);
    m_lobby.SetReady(from, true);
}

void PacketHandler::OnCharacterSelected(char *buffer, size_t size, network::PeerId from) {
    auto *pkt = reinterpret_cast<network::CharacterSelectedPacket *>(buffer);
    bool characterSet = m_lobby.TrySetCharacter(from, pkt->characterId);
    if (!characterSet) {
        network::CharacterSelectedPacket deniedPacket;
        deniedPacket.header.type = network::PacketType::CharacterSelected;
        deniedPacket.playerId = pkt->playerId;
        deniedPacket.characterId = Character::CharacterId::None;
        m_transport.send(from, &deniedPacket, sizeof(deniedPacket));
        return;
    }

    m_broadcaster.BroadcastCharacterSelected(pkt->playerId, pkt->characterId);
}

void PacketHandler::OnDisconnect(char *buffer, network::PeerId from) {
    auto *client = m_registry.FindByPeer(from);
    if (!client)
        return;

    m_lobby.RemovePlayer(from);
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
