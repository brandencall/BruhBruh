#pragma once

#include "../shared/events.hpp"
#include <vector>

class EventBus {
  public:
    // Bullet Events
    void publish(const event::BulletSpawnEvent &e) { m_bulletSpawnEvents.push_back(e); }
    void publish(const event::BulletHitEvent &e) { m_bulletHitEvents.push_back(e); }
    void publish(const event::BulletExpireEvent &e) { m_bulletExpireEvents.push_back(e); }

    // Player Events
    void publish(const event::PlayerDiedEvent &e) { m_playerDiedEvents.push_back(e); }

    // Wall Events
    void publish(const event::PlaceWallEvent &e) { m_placeWallEvents.push_back(e); }
    void publish(const event::DestroyWallEvent &e) { m_destroyWallEvents.push_back(e); }

    // Drain bullet events
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

    // Drain player events
    template <typename Func> void DrainPlayerDeath(Func callback) {
        for (const auto &e : m_playerDiedEvents)
            callback(e);
    }

    // Drain wall events
    template <typename Func> void DrainPlaceWall(Func callback) {
        for (const auto &e : m_placeWallEvents)
            callback(e);
    }
    template <typename Func> void DrainDestroyWall(Func callback) {
        for (const auto &e : m_destroyWallEvents)
            callback(e);
    }

    void clear() {
        m_bulletSpawnEvents.clear();
        m_bulletHitEvents.clear();
        m_bulletExpireEvents.clear();
        m_playerDiedEvents.clear();
        m_placeWallEvents.clear();
        m_destroyWallEvents.clear();
    }

  private:
    // Bullet Events
    std::vector<event::BulletSpawnEvent> m_bulletSpawnEvents;
    std::vector<event::BulletHitEvent> m_bulletHitEvents;
    std::vector<event::BulletExpireEvent> m_bulletExpireEvents;

    // Player Events
    std::vector<event::PlayerDiedEvent> m_playerDiedEvents;

    // Wall Events
    std::vector<event::PlaceWallEvent> m_placeWallEvents;
    std::vector<event::DestroyWallEvent> m_destroyWallEvents;
};
