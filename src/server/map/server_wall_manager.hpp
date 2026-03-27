#pragma once

#include "../event_bus.hpp"
#include "map/grid.hpp"
#include "map/wall_manager.hpp"
#include <cstdint>
#include <functional>

namespace Map {

class ServerWallManager : public WallManager {
  public:
    ServerWallManager() = default;

    void Initialize(EventBus &eventBus) { m_eventBus = &eventBus; }

    void SetOnWallPlaced(std::function<void(Map::Vector2i, float, uint32_t)> callback) {
        m_onWallPlaced = std::move(callback);
    }

    void SetOnWallDamaged(std::function<void(Map::Vector2i, float, uint32_t)> callback) {
        m_onWallDamaged = std::move(callback);
    }

    void SetOnWallDestroyed(std::function<void(Map::Vector2i, uint32_t)> callback) {
        m_onWallDestroyed = std::move(callback);
    }

  protected:
    void OnWallPlaced(Map::Vector2i gridPos, float health, uint32_t ownerId) override {
        if (m_onWallPlaced)
            m_onWallPlaced(gridPos, health, ownerId);
    }

    void OnWallDamaged(Map::Vector2i gridPos, float currentHealth, uint32_t ownerId) override {
        if (m_onWallDamaged)
            m_onWallDamaged(gridPos, currentHealth, ownerId);
    }

    void OnWallDestroyed(Map::Vector2i gridPos, uint32_t ownerId) override {
        if (m_onWallDestroyed)
            m_onWallDestroyed(gridPos, ownerId);
    }

  private:
    EventBus *m_eventBus = nullptr;

    std::function<void(Map::Vector2i gridPos, float health, uint32_t ownerId)> m_onWallPlaced;
    std::function<void(Map::Vector2i gridPos, float currentHealth, uint32_t ownerId)> m_onWallDamaged;
    std::function<void(Map::Vector2i gridPos, uint32_t ownerId)> m_onWallDestroyed;
};

} // namespace Map
