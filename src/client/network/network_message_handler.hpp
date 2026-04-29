#pragma once
#include "../../network/packets/packet_header.hpp"
#include <functional>
#include <unordered_map>

class NetworkMessageHandler {
  public:
    using HandlerFn = std::function<void(const char *buf)>;

    void Register(network::PacketType type, HandlerFn fn) { m_handlers[type] = std::move(fn); }

    void Unregister(network::PacketType type) { m_handlers.erase(type); }

    void Dispatch(const char *buf, size_t size) {
        if (size < sizeof(network::PacketHeader))
            return;

        auto *header = reinterpret_cast<const network::PacketHeader *>(buf);

        auto it = m_handlers.find(header->type);
        if (it != m_handlers.end())
            it->second(buf);
    }

  private:
    std::unordered_map<network::PacketType, HandlerFn> m_handlers;
};
