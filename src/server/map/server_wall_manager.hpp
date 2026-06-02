#pragma once

#include "../../shared/map/dynamic_walls/wall_manager.hpp"
#include "../../shared/map/grid.hpp"
#include "../../shared/state/player_state.hpp"
#include "../event_bus.hpp"
#include <cstdint>

namespace Map {

class ServerWallManager : public WallManager {
  public:
    ServerWallManager(std::array<state::PlayerState, MAX_PLAYERS> &players) : m_players(players) {}

    void Initialize(EventBus &eventBus) { m_eventBus = &eventBus; }

  protected:
    void OnWallPlaced(Map::Vector2i gridPos, float health, const state::PlayerState &player) override {
        m_eventBus->publish(event::PlaceWallEvent{gridPos, health, player});
    }

    void OnWallDamaged(Map::Vector2i gridPos, float currentHealth, uint32_t ownerId) override {
        m_eventBus->publish(event::DamageWallEvent{gridPos, currentHealth, ownerId});
    }

    void OnWallDestroyed(Map::Vector2i gridPos, uint32_t ownerId) override {
        state::PlayerState &player = m_players[ownerId];
        player.currentAvaliableWalls++;
        m_eventBus->publish(event::DestroyWallEvent{gridPos, player});
    }

    void OnWallPickedUp(Map::Vector2i gridPos, uint32_t ownerId) override {
        state::PlayerState &player = m_players[ownerId];
        player.currentAvaliableWalls++;
        m_eventBus->publish(event::WallPickedUpEvent{gridPos, player});
    }

    void OnWallInputDenied(uint32_t playerId) const override {
        m_eventBus->publish(event::WallInputDeniedEvent{playerId});
    }

  private:
    std::array<state::PlayerState, MAX_PLAYERS> &m_players;
    EventBus *m_eventBus = nullptr;
};

} // namespace Map
