#pragma once
#include "subscription.hpp"
#include <cstdint>
#include <functional>
#include <unordered_map>

namespace Client {

template <typename TEvent> class EventBus {
  public:
    using Handler = std::function<void(const TEvent &)>;
    using Token = uint32_t;

    Subscription Subscribe(Handler handler) {
        Token token = m_nextToken++;

        m_subscribers.emplace(token, std::move(handler));

        // RAII unsubscribe
        return Subscription([this, token]() { Unsubscribe(token); });
    }

    void Unsubscribe(Token token) { m_subscribers.erase(token); }

    void Publish(const TEvent &event) {
        auto copy = m_subscribers;

        for (auto &[token, handler] : copy) {
            handler(event);
        }
    }

  private:
    std::unordered_map<Token, Handler> m_subscribers;
    Token m_nextToken = 0;
};

} // namespace Client
