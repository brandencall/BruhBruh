#pragma once
#include <functional>

namespace Client {

class Subscription {
  public:
    Subscription() = default;

    Subscription(std::function<void()> fn) : m_unsubscribe(std::move(fn)) {}

    ~Subscription() {
        if (m_unsubscribe)
            m_unsubscribe();
    }

    Subscription(const Subscription &) = delete;
    Subscription &operator=(const Subscription &) = delete;

    Subscription(Subscription &&other) noexcept {
        m_unsubscribe = std::move(other.m_unsubscribe);
        other.m_unsubscribe = nullptr;
    }

    Subscription &operator=(Subscription &&other) noexcept {
        if (this != &other) {
            if (m_unsubscribe)
                m_unsubscribe();
            m_unsubscribe = std::move(other.m_unsubscribe);
            other.m_unsubscribe = nullptr;
        }
        return *this;
    }

  private:
    std::function<void()> m_unsubscribe;
};

} // namespace Client
