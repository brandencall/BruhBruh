#pragma once
#include "../shared/events.hpp"
#include <vector>

class EventBus {
  public:
    // Bullet Events
    void publish(const event::BulletSpawnEvent &e) { m_bulletSpawnEvents.push_back(e); }
    void publish(const event::BulletDestroyedEvent &e) { m_bulletDestroyedEvents.push_back(e); }

    // Player Events
    void publish(const event::PlayerRespawnEvent &e) { m_playerRespawnEvents.push_back(e); }
    void publish(const event::PlayerDamagedEvent &e) { m_playerDamagedEvents.push_back(e); }
    void publish(const event::PlayerDiedEvent &e) { m_playerDiedEvents.push_back(e); }

    // Wall Events
    void publish(const event::PlaceWallEvent &e) { m_placeWallEvents.push_back(e); }
    void publish(const event::DamageWallEvent &e) { m_damageWallEvents.push_back(e); }
    void publish(const event::DestroyWallEvent &e) { m_destroyWallEvents.push_back(e); }
    void publish(const event::WallPickedUpEvent &e) { m_wallPickedUpEvents.push_back(e); }

    // Powerup Events
    void publish(const event::PowerUpSpawnEvent &e) { m_powerUpSpawnEvents.push_back(e); }

    // Drain bullet events
    template <typename Func> void DrainBulletSpawn(Func callback) {
        for (const auto &e : m_bulletSpawnEvents)
            callback(e);
    }
    template <typename Func> void DrainBulletDestroyed(Func callback) {
        for (const auto &e : m_bulletDestroyedEvents)
            callback(e);
    }

    // Drain player events
    template <typename Func> void DrainPlayerRespawn(Func callback) {
        for (const auto &e : m_playerRespawnEvents)
            callback(e);
    }
    template <typename Func> void DrainPlayerDamaged(Func callback) {
        for (const auto &e : m_playerDamagedEvents)
            callback(e);
    }
    template <typename Func> void DrainPlayerDeath(Func callback) {
        for (const auto &e : m_playerDiedEvents)
            callback(e);
    }

    // Drain wall events
    template <typename Func> void DrainPlaceWall(Func callback) {
        for (const auto &e : m_placeWallEvents)
            callback(e);
    }
    template <typename Func> void DrainDamageWall(Func callback) {
        for (const auto &e : m_damageWallEvents)
            callback(e);
    }
    template <typename Func> void DrainDestroyWall(Func callback) {
        for (const auto &e : m_destroyWallEvents)
            callback(e);
    }

    template <typename Func> void DrainWallPickedUp(Func callback) {
        for (const auto &e : m_wallPickedUpEvents)
            callback(e);
    }

    // Drain powerup events
    template <typename Func> void DrainPowerUpSpawn(Func callback) {
        for (const auto &e : m_powerUpSpawnEvents)
            callback(e);
    }

    void clear() {
        m_bulletSpawnEvents.clear();
        m_bulletDestroyedEvents.clear();
        m_playerRespawnEvents.clear();
        m_playerDamagedEvents.clear();
        m_playerDiedEvents.clear();
        m_placeWallEvents.clear();
        m_damageWallEvents.clear();
        m_destroyWallEvents.clear();
        m_wallPickedUpEvents.clear();
        m_powerUpSpawnEvents.clear();
    }

  private:
    // Bullet Events
    std::vector<event::BulletSpawnEvent> m_bulletSpawnEvents;
    std::vector<event::BulletDestroyedEvent> m_bulletDestroyedEvents;

    // Player Events
    std::vector<event::PlayerRespawnEvent> m_playerRespawnEvents;
    std::vector<event::PlayerDamagedEvent> m_playerDamagedEvents;
    std::vector<event::PlayerDiedEvent> m_playerDiedEvents;

    // Wall Events
    std::vector<event::PlaceWallEvent> m_placeWallEvents;
    std::vector<event::DamageWallEvent> m_damageWallEvents;
    std::vector<event::DestroyWallEvent> m_destroyWallEvents;
    std::vector<event::WallPickedUpEvent> m_wallPickedUpEvents;

    // Powerup Events
    std::vector<event::PowerUpSpawnEvent> m_powerUpSpawnEvents;
};
