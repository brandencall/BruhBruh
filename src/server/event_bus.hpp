#pragma once

#include "../shared/events.hpp"
#include <vector>

class EventBus {
  public:
    void publish(const event::BulletSpawnEvent &e) { m_bulletSpawnEvents.push_back(e); }
    void publish(const event::BulletHitEvent &e) { m_bulletHitEvents.push_back(e); }
    void publish(const event::BulletExpireEvent &e) { m_bulletExpireEvents.push_back(e); }
    void publish(const event::PlayerDiedEvent &e) { m_playerDiedEvents.push_back(e); }

    template <typename Func> void DrainBulletSpawn(Func callback) {
        for (const auto &e : m_bulletSpawnEvents)
            callback(e);
    }

    template <typename Func> void DrainBulletHit(Func callback) {
        for (const auto &e : m_bulletHitEvents)
            callback(e);
    }

    template <typename Func> void DrainBulletExpire(Func callback) {
        for (const auto &e : m_bulletExpireEvents)
            callback(e);
    }

    template <typename Func> void DrainPlayerDeath(Func callback) {
        for (const auto &e : m_playerDiedEvents)
            callback(e);
    }

    void clear() {
        m_bulletSpawnEvents.clear();
        m_bulletHitEvents.clear();
        m_bulletExpireEvents.clear();
        m_playerDiedEvents.clear();
    }

  private:
    std::vector<event::BulletSpawnEvent> m_bulletSpawnEvents;
    std::vector<event::BulletHitEvent> m_bulletHitEvents;
    std::vector<event::BulletExpireEvent> m_bulletExpireEvents;

    std::vector<event::PlayerDiedEvent> m_playerDiedEvents;
};
