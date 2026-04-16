#include "packet_handler.hpp"
#include "../network/packets/gameplay_packets.hpp"
#include "../network/packets/lobby_packets.hpp"
#include "characters/character_types.hpp"
#include "packet_handler.hpp"

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
    if (m_phase != ServerPhase::LOBBY || existing)
        return;

    auto *pkt = reinterpret_cast<network::JoinLobbyPacket *>(buffer);
    auto *client = m_registry.AddClient(from);
    if (!client) {
        // TODO: Send a lobby full packet so that the client knows that it is getting denied
        return;
    }

    // TODO: need to pass in the name when the client joins
    // int slot = m_lobby.AddPlayer(from, name, client->playerId);
    int slot = m_lobby.AddPlayer(from, client->playerId);
    SendJoinResponse(from, client->playerId);

    m_broadcaster.BroadcastPlayerJoined(m_lobby.Slots()[slot].lobbySlot.name, client);
}

void PacketHandler::OnPlayerReady(char *buffer, size_t size, network::PeerId from) {
    auto *pkt = reinterpret_cast<network::PlayerReadyPacket *>(buffer);
    bool setReady = m_lobby.TrySetReady(from, pkt->playerReady);

    network::PlayerReadyPacket returnPacket{};
    returnPacket.header.type = network::PacketType::PlayerReady;
    returnPacket.playerReady = setReady;
    returnPacket.playerId = pkt->playerId;
    returnPacket.characterId = pkt->characterId;
    m_transport.send(from, &returnPacket, sizeof(returnPacket));
}

void PacketHandler::OnCharacterSelected(char *buffer, size_t size, network::PeerId from) {
    auto *pkt = reinterpret_cast<network::CharacterSelectedPacket *>(buffer);
    m_lobby.TrySetCharacter(from, pkt->characterId);
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
                                .angle = packet->facingAngle,
                                .buttons = packet->buttons,
                            });
}

void PacketHandler::SendJoinResponse(network::PeerId to, uint32_t playerId) {
    network::JoinResponsePacket response{};
    response.header.type = network::PacketType::JoinResponse;
    response.playerId = playerId;
    response.characterId = Character::CharacterId::None;
    m_transport.send(to, &response, sizeof(response));
}
} // namespace network
