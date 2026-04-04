#pragma once
#include <cstdint>
#include <functional>
#include <unordered_map>

namespace Client {
using SubscriptionToken = uint32_t;

template <typename TEvent> class EventBus {
  public:
    using Handler = std::function<void(const TEvent &)>;

    SubscriptionToken Subscribe(Handler handler) {
        SubscriptionToken token = m_nextToken++;
        m_subscribers[token] = std::move(handler);
        return token;
    }

    void Unsubscribe(SubscriptionToken token) { m_subscribers.erase(token); }

    void Publish(const TEvent &event) {
        for (auto &[token, handler] : m_subscribers)
            handler(event);
    }

  private:
    std::unordered_map<SubscriptionToken, Handler> m_subscribers;
    SubscriptionToken m_nextToken = 0;
};
} // namespace Client
