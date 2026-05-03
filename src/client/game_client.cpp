#include "game_client.hpp"
#include "../network/packets/lobby_packets.hpp"
#include "network/ITransport.hpp"
#include "network/network_message_handler.hpp"
#include "network/steam_lobby_manager.hpp"
#include <iostream>

GameClient::GameClient(network::ITransport &transport, SteamLobbyManager &lobbyManager, NetworkMessageHandler &handler)
    : m_transport(&transport), m_lobbyManager(lobbyManager), m_handler(handler) {}

void GameClient::Start(const char *ip, int port) {
    m_ownedTransport.connect(ip, port);
    // TODO: NEED TO FIX THIS LIKE IN GAMESERVER
    m_transport = &m_ownedTransport;
    m_running = true;
    while (m_running) {
        Update();
    }
}

void GameClient::StartInProcess() { m_running = true; }

void GameClient::Update() {
    network::InboundPacket pkt;
    while (m_running && m_transport->recvClient(pkt)) {
        m_handler.Dispatch(pkt.data, pkt.size);
    }
}

void GameClient::Disconnect() {
    std::cout << "In game client disconnect" << std::endl;
    network::DisconnectPacket packet{};
    packet.header.type = network::PacketType::Disconnect;
    m_transport->send(network::PEER_SERVER, &packet, sizeof(packet));
}
