#pragma once

#include "../../config.hpp"
#include "../components/collision.hpp"
#include "../events.hpp"
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
    bool PlaceWall(Vector2 worldPos, uint32_t ownerId, float health, const std::vector<Collision::AABB> &staticWalls,
                   const std::array<state::PlayerState, MAX_PLAYERS> &players) {
        Vector2i gridPos = WorldToGrid(worldPos);
        if (CanPlaceWall(gridPos, staticWalls, players)) {
            m_walls[gridPos] = DynamicWall{
                .gridPos = gridPos, .health = health, .maxHealth = health, .ownerId = ownerId, .active = true};
            m_placeWallEvents.emplace_back(worldPos, health, ownerId);

            return true;
        }
        // Add wall deny event (MAYBE??)
        return false;
    };

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

    // Returns true if wall was destroyed
    bool DamageWall(Vector2i gridPos, float damage) { return false; }

    void RemoveWall(Vector2i gridPos) {}

    const DynamicWall *GetWall(Vector2i gridPos) const { return nullptr; }

    const auto &GetAllWalls() const { return m_walls; }

    void clearEvents() { m_placeWallEvents.clear(); }

  public:
    std::vector<event::PlaceWallEvent> m_placeWallEvents;

  private:
    std::unordered_map<Vector2i, DynamicWall, GridHash> m_walls;
};

} // namespace Map
