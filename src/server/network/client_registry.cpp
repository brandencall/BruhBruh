#include "client_registry.hpp"

namespace network {

network::ClientConnection *ClientRegistry::FindByPeer(network::PeerId id) {
    for (auto &client : m_clients) {
        if (client.active && client.peerId == id)
            return &client;
    }
    return nullptr;
}

network::ClientConnection *ClientRegistry::AddClient(network::PeerId id, Character::CharacterId character) {
    for (int i = 0; i < m_clients.size(); ++i) {
        if (!m_clients[i].active) {
            m_clients[i].active = true;
            m_clients[i].peerId = id;
            m_clients[i].playerId = i;
            m_clients[i].characterId = character;
            return &m_clients[i];
        }
    }
    return nullptr; // server full
}

void ClientRegistry::RemoveClient(network::PeerId id) {
    auto *client = FindByPeer(id);
    if (client)
        *client = network::ClientConnection{}; // reset to default
}

} // namespace network
