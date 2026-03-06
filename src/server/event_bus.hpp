#pragma once

#include "../shared/events.hpp"
#include <vector>

class EventBus {
  public:
    void publish(const event::BulletSpawnEvent &e) { m_spawnEvents.push_back(e); }
    void publish(const event::BulletHitEvent &e) { m_hitEvents.push_back(e); }
    void publish(const event::BulletExpireEvent &e) { m_expireEvents.push_back(e); }

    template <typename Func> void drainSpawn(Func callback) {
        for (const auto &e : m_spawnEvents)
            callback(e);
    }

    template <typename Func> void drainHit(Func callback) {
        for (const auto &e : m_hitEvents)
            callback(e);
    }

    template <typename Func> void drainExpire(Func callback) {
        for (const auto &e : m_expireEvents)
            callback(e);
    }

    void clear() {
        m_spawnEvents.clear();
        m_hitEvents.clear();
        m_expireEvents.clear();
    }

  private:
    std::vector<event::BulletSpawnEvent> m_spawnEvents;
    std::vector<event::BulletHitEvent> m_hitEvents;
    std::vector<event::BulletExpireEvent> m_expireEvents;
};
