// shared/net/ITransport.h
#pragma once
#include <cstdint>
#include <stddef.h>

namespace network {

using PeerId = uint32_t;
constexpr PeerId PEER_SERVER = 0;

struct InboundPacket {
    PeerId from;
    char *data;
    size_t size;
};

class ITransport {
  public:
    virtual ~ITransport() = default;
    virtual bool send(PeerId to, const void *data, size_t size) = 0;
    virtual bool recv(InboundPacket &out) = 0;
};

} // namespace network
