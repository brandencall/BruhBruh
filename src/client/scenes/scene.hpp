#pragma once
#include "../event_bus.hpp"
#include "scene_manager_fwd.hpp"
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
        m_subscriptions.push_back(bus.Subscribe(std::move(handler)));
    }

  private:
    void UnsubscribeAll() { m_subscriptions.clear(); }
    std::vector<Client::Subscription> m_subscriptions;
};
