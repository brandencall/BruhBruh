#pragma once
#include "../network/server.hpp"
#include "../shared/network/ITransport.hpp"
#include <unordered_map>

namespace network {

class ServerTransport : public ITransport {
  public:
    bool start(uint16_t port) { return m_server.Start(port); }

    bool send(PeerId to, const void *data, size_t size) override {
        auto it = m_peers.find(to);
        if (it == m_peers.end())
            return false;
        m_server.Send(static_cast<const char *>(data), size, it->second);
        return true;
    }

    bool recv(InboundPacket &out) override {
        sockaddr_in addr{};
        int received = m_server.Receive(m_buffer, sizeof(m_buffer), addr);
        if (received <= 0)
            return false;

        out.from = getOrRegisterPeer(addr);
        out.data = m_buffer;
        out.size = static_cast<size_t>(received);
        return true;
    }

  private:
    PeerId getOrRegisterPeer(const sockaddr_in &addr) {
        // Check if we've seen this address before
        for (auto &[id, stored] : m_peers) {
            if (stored.sin_addr.s_addr == addr.sin_addr.s_addr && stored.sin_port == addr.sin_port) {
                return id;
            }
        }
        // New peer — assign next ID
        PeerId newId = m_nextPeerId++;
        m_peers[newId] = addr;
        return newId;
    }

    Server m_server;
    char m_buffer[2048];
    uint32_t m_nextPeerId = 1; // 0 is reserved for PEER_SERVER

    std::unordered_map<PeerId, sockaddr_in> m_peers;
};

} // namespace network
