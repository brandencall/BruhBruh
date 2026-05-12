#pragma once
#include "../../shared/systems/bullet_system.hpp"
#include "../event_bus.hpp"
#include "raylib.h"
#include <cstdint>
#include <functional>

namespace System {

class ServerBulletSystem : public BulletSystem<state::BulletState> {
  public:
    ServerBulletSystem() = default;

    void Initialize(EventBus &eventBus) { m_eventBus = &eventBus; }

    void SetOnWallHit(std::function<void(Map::Vector2i, float, uint32_t)> callback) {
        m_onWallHit = std::move(callback);
    }

    void SetOnPlayerHit(std::function<void(uint32_t, float, uint32_t)> callback) {
        m_onPlayerHit = std::move(callback);
    }

    void SetOnBulletSpawn(
        std::function<void(uint32_t, uint32_t, Character::CharacterId, Vector2, Vector2, uint32_t)> callback) {
        m_onBulletSpawn = std::move(callback);
    }

    void SetOnBulletDestroyed(std::function<void(int, Vector2)> callback) { m_onBulletDestroyed = std::move(callback); }

  protected:
    void OnWallHit(Map::Vector2i gridPos, float damage, uint32_t shooterId) override {
        if (m_onWallHit)
            m_onWallHit(gridPos, damage, shooterId);
    }
    void OnPlayerHit(uint32_t playerId, float damage, uint32_t shooterId) override {
        if (m_onPlayerHit)
            m_onPlayerHit(playerId, damage, shooterId);
    }
    void OnBulletSpawn(uint32_t bulletId, uint32_t ownerId, Character::CharacterId characterId, Vector2 position,
                       Vector2 velocity, uint32_t bulletPredSequence) override {
        if (m_onBulletSpawn)
            m_onBulletSpawn(bulletId, ownerId, characterId, position, velocity, bulletPredSequence);
    }
    void OnBulletDestroyed(int slot, Vector2 position) override {
        m_bullets[slot].active = false;
        if (m_onBulletDestroyed)
            m_onBulletDestroyed(slot, position);
    }

  private:
    EventBus *m_eventBus = nullptr;

    std::function<void(Map::Vector2i gridPos, float damage, uint32_t shooterId)> m_onWallHit;
    std::function<void(uint32_t playerId, float damage, uint32_t shooterId)> m_onPlayerHit;
    std::function<void(uint32_t bulletId, uint32_t ownerId, Character::CharacterId characterId, Vector2 position,
                       Vector2 velocity, uint32_t bulletPredSequence)>
        m_onBulletSpawn;
    std::function<void(uint32_t bulletId, Vector2 position)> m_onBulletDestroyed;
};

} // namespace System
