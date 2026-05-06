#include "game_client.hpp"
#include "../network/packets/lobby_packets.hpp"
#include "network/ITransport.hpp"
#include "network/network_message_handler.hpp"

GameClient::GameClient(network::ITransport &transport, NetworkMessageHandler &handler)
    : m_transport(&transport), m_handler(handler) {}

GameClient::GameClient(NetworkMessageHandler &handler) : m_handler(handler) {}

void GameClient::Start(const char *ip, int port) {
    m_ownedTransport.connect(ip, port);
    m_transport = &m_ownedTransport;
    m_running = true;
}

void GameClient::StartInProcess() { m_running = true; }

void GameClient::Update() {
    network::InboundPacket pkt;
    while (m_running && m_transport->recvClient(pkt)) {
        m_handler.Dispatch(pkt.data, pkt.size);
    }
}

void GameClient::Disconnect() {
    network::DisconnectPacket packet{};
    packet.header.type = network::PacketType::Disconnect;
    m_transport->send(network::PEER_SERVER, &packet, sizeof(packet));
}

network::ITransport *GameClient::GetTransport() { return m_transport; }
