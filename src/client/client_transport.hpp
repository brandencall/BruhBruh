#pragma once
#include "../network/client.hpp"
#include "../shared/network/ITransport.hpp"

namespace network {

class ClientTransport : public ITransport {
  public:
    bool connect(const char *host, int port) { return m_client.Connect(host, port); }

    // PeerId 'to' is ignored — client always sends to server
    bool send(PeerId to, const void *data, size_t size) override {
        m_client.Send(static_cast<const char *>(data), size);
        return true;
    }

    bool recv(InboundPacket &out) override {
        int received = m_client.Receive(out.data, sizeof(MAX_PACKET_SIZE));
        if (received <= 0)
            return false;

        out.from = PEER_SERVER;
        out.size = static_cast<size_t>(received);
        return true;
    }

  private:
    Client m_client;
};

} // namespace network
