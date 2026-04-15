#pragma once

#include "../config.hpp"
#include "../network/client.hpp"
#include <array>

namespace network {
class ClientRegistry {
  public:
    network::ClientConnection *FindByPeer(network::PeerId id);
    network::ClientConnection *AddClient(network::PeerId id);
    void RemoveClient(network::PeerId id);

    template <typename F> void ForEach(F &&fn) {
        for (auto &client : m_clients) {
            if (client.active)
                fn(client);
        }
    }

  private:
    std::array<network::ClientConnection, MAX_PLAYERS> m_clients;
};
} // namespace network
