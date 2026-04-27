// shared/net/ITransport.h
#pragma once
#include <cstdint>
#include <stddef.h>

namespace network {

using PeerId = uint32_t;
constexpr PeerId PEER_SERVER = 0;
constexpr int MAX_PACKET_SIZE = 2048;

struct InboundPacket {
    PeerId from;
    char data[MAX_PACKET_SIZE];
    size_t size;
};

class ITransport {
  public:
    virtual ~ITransport() = default;
    virtual bool send(PeerId to, const void *data, size_t size) = 0;
    virtual bool recv(InboundPacket &out) { return false; };
    virtual void Pump() {}
    virtual bool recvServer(InboundPacket &out) { return recv(out); }
    virtual bool recvClient(InboundPacket &out) { return recv(out); }
};

} // namespace network
