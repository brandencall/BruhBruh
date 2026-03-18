#pragma once

#include "../../config.hpp"
#include "../components/collision.hpp"
#include "../events.hpp"
#include "./grid.hpp"
#include "dynamic_wall.hpp"
#include <array>
#include <unordered_map>
#include <vector>

namespace Map {

// Hash for grid position lookup
struct GridHash {
    size_t operator()(const Vector2i &v) const { return std::hash<int>()(v.x) ^ (std::hash<int>()(v.y) << 16); }
};

class WallManager {
  public:
    explicit WallManager(bool trackEvents = false) : m_trackEvents(trackEvents) {}

    void PlaceWall(Map::Vector2i gridPos, float health, uint32_t ownerId) {
        m_walls[gridPos] = DynamicWall{.gridPos = gridPos,
                                       .health = health,
                                       .maxHealth = health,
                                       .ownerId = ownerId,
                                       .collider = GridCellToAABB(gridPos),
                                       .active = true};
        if (m_trackEvents) {
            m_placeWallEvents.emplace_back(gridPos, health, ownerId);
        }
    }

    bool CanPlaceWall(Vector2i gridPos, const std::vector<Collision::AABB> &staticWalls,
                      const std::array<state::PlayerState, MAX_PLAYERS> &players) const {

        Collision::AABB newWallAABB = GridCellToAABB(gridPos);

        // Check position with static walls
        for (const auto &wall : staticWalls) {
            if (Collision::Overlap(newWallAABB, wall)) {
                return false;
            }
        }
        // Check position with players
        for (const auto &player : players) {
            if (Collision::Overlap(Collision::HurtboxToCircle(player.position, player.hurtbox), newWallAABB)) {
                return false;
            }
        }
        // Check position with m_walls
        for (const auto &wall : m_walls) {
            if (Collision::Overlap(newWallAABB, GridCellToAABB(wall.second.gridPos))) {
                return false;
            }
        }
        return true;
    };

    void UpdateWallHealth(Vector2i gridPos, float currentHealth) {
        if (m_walls.find(gridPos) != m_walls.end()) {
            m_walls[gridPos].health = currentHealth;
        }
    }

    // Returns true if wall was destroyed
    bool DamageWall(Vector2i gridPos, float damage) { return false; }

    void RemoveWall(Vector2i gridPos) {
        auto it = m_walls.find(gridPos);
        if (it != m_walls.end())
            m_walls.erase(it);
    }

    const DynamicWall *GetWall(Vector2i gridPos) const { return nullptr; }

    const auto &GetAllWalls() const { return m_walls; }

    auto &GetAllWalls() { return m_walls; }

    const std::vector<Collision::AABB> GetColliders() const {
        std::vector<Collision::AABB> colliders;
        colliders.reserve(m_walls.size());
        for (const auto &[gridPos, wall] : m_walls) {
            if (wall.active)
                colliders.push_back(wall.collider);
        }
        return colliders;
    }

    void clearEvents() { m_placeWallEvents.clear(); }

  public:
    std::vector<event::PlaceWallEvent> m_placeWallEvents;

  private:
    bool m_trackEvents;
    std::unordered_map<Vector2i, DynamicWall, GridHash> m_walls;
};

} // namespace Map
