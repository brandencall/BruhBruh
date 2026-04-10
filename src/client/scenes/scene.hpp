#pragma once
#include "../event_bus.hpp"
#include "scene_manager_fwd.hpp"
#include <functional>
#include <vector>

class Scene {
  public:
    virtual ~Scene() { UnsubscribeAll(); }

    virtual void OnEnter() = 0;
    virtual void OnExit() { UnsubscribeAll(); }
    virtual void Update(float dt) = 0;
    virtual void Render() = 0;

  protected:
    template <typename TEvent>
    void Subscribe(Client::EventBus<TEvent> &bus, typename Client::EventBus<TEvent>::Handler handler) {
        auto token = bus.Subscribe(std::move(handler));
        m_unsubscribers.push_back([&bus, token]() { bus.Unsubscribe(token); });
    }

  private:
    void UnsubscribeAll() {
        for (auto &fn : m_unsubscribers)
            fn();
        m_unsubscribers.clear();
    }

    std::vector<std::function<void()>> m_unsubscribers;
};
