#pragma once

#include "../../config.hpp"
#include "../../state/player_state.hpp"
#include "../dynamic_walls/dynamic_wall.hpp"
#include "../grid.hpp"
#include <array>
#include <cstdint>
#include <sys/types.h>
#include <unordered_map>
#include <vector>

namespace Map {

// Hash for grid position lookup
struct GridHash {
    size_t operator()(const Vector2i &v) const { return std::hash<int>()(v.x) ^ (std::hash<int>()(v.y) << 16); }
};

class WallManager {
  public:
    WallManager() = default;

    void HandleWallInput(state::PlayerState &player, const state::PlayerInput &input,
                         const Character::CharacterDef &charDef, const std::vector<Collision::AABB> &staticWalls,
                         const std::array<state::PlayerState, MAX_PLAYERS> &players) {
        const Map::Vector2i gridPos = Map::WorldToGrid({input.aimX, input.aimY});
        if (TryPlaceWall(player, gridPos, staticWalls, players)) {
            player.wallTimer = charDef.wallCooldown;
            return;
        }

        if (PickUpWall(gridPos, player)) {
            player.wallTimer = charDef.wallCooldown;
            return;
        }

        OnWallInputDenied(player.id);
    }

    bool TryPlaceWall(state::PlayerState &player, Map::Vector2i gridPos,
                      const std::vector<Collision::AABB> &staticWalls,
                      const std::array<state::PlayerState, MAX_PLAYERS> &players) {

        if (player.wallTimer > 0.0f)
            return false;

        if (!CanPlaceWall(gridPos, staticWalls, player, players))
            return false;

        player.currentAvaliableWalls--;
        PlaceWall(gridPos, 50, player);
        return true;
    }

    void PlaceWall(const Map::Vector2i &gridPos, float health, const state::PlayerState &player,
                   float spawnTime = 0.0f) {
        m_walls[gridPos] = DynamicWall{.gridPos = gridPos,
                                       .health = health,
                                       .maxHealth = health,
                                       .ownerId = player.id,
                                       .ownerCharacter = player.characterId,
                                       .collider = GridCellToAABB(gridPos),
                                       .spawnTime = spawnTime,
                                       .active = true};
        OnWallPlaced(gridPos, health, player);
    }

    bool CanPlaceWall(const Vector2i &gridPos, const std::vector<Collision::AABB> &staticWalls,
                      const state::PlayerState &currentPlayer,
                      const std::array<state::PlayerState, MAX_PLAYERS> &players) const {
        Collision::AABB newWallAABB = GridCellToAABB(gridPos);

        // Check position with static walls
        for (const auto &wall : staticWalls) {
            if (Collision::Overlap(newWallAABB, wall))
                return false;
        }
        // Check position with players
        for (const auto &player : players) {
            if (player.active &&
                Collision::Overlap(Collision::HurtboxToCircle(player.position, player.hurtbox), newWallAABB))
                return false;
        }
        // Check position with m_walls
        for (const auto &wall : m_walls) {
            if (Collision::Overlap(newWallAABB, GridCellToAABB(wall.second.gridPos)))
                return false;
        }

        return currentPlayer.currentAvaliableWalls > 0;
    };

    void UpdateWallHealth(const Vector2i &gridPos, float currentHealth) {
        if (m_walls.find(gridPos) != m_walls.end()) {
            m_walls[gridPos].health = currentHealth;
        }
    }

    // Returns true if it damaged the wall
    bool DamageWall(const Vector2i &gridPos, float damage, uint32_t shooterId) {
        if (m_walls.find(gridPos) == m_walls.end() || m_walls[gridPos].ownerId == shooterId)
            return false;

        DynamicWall &wall = m_walls[gridPos];
        wall.health -= damage;
        if (wall.health <= 0.0f) {
            DestroyWall(gridPos, wall.ownerId);
            return true;
        }
        OnWallDamaged(gridPos, wall.health, wall.ownerId);
        return true;
    }

    bool PickUpWall(const Vector2i &gridPos, const state::PlayerState &currentPlayer) {
        if (currentPlayer.wallTimer > 0.0f)
            return false;

        auto it = m_walls.find(gridPos);
        if (it == m_walls.end() || it->second.ownerId != currentPlayer.id)
            return false;

        m_walls.erase(it);
        OnWallPickedUp(gridPos, currentPlayer.id);
        return true;
    }

    void DestroyWall(const Vector2i &gridPos, uint32_t ownerId) {
        auto it = m_walls.find(gridPos);
        if (it == m_walls.end())
            return;

        m_walls.erase(it);
        OnWallDestroyed(gridPos, ownerId);
        return;
    }

    int GetOwnerId(const Vector2i &gridPos) {
        auto it = m_walls.find(gridPos);
        if (it == m_walls.end())
            return -1;

        return it->second.ownerId;
    }

    void SetWalls(std::unordered_map<Vector2i, DynamicWall, GridHash> walls) { m_walls = walls; }

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

    void ClearWallsForPlayer(uint32_t ownerId) {
        for (auto it = m_walls.begin(); it != m_walls.end();) {
            if (it->second.ownerId == ownerId) {
                OnWallDestroyed(it->first, ownerId);
                it = m_walls.erase(it);
            } else {
                ++it;
            }
        }
    }

    void Reset() { m_walls.clear(); }

  protected:
    virtual void OnWallPlaced(Map::Vector2i gridPos, float health, const state::PlayerState &player) {}
    virtual void OnWallDamaged(Map::Vector2i gridPos, float currentHealth, uint32_t ownerId) {}
    virtual void OnWallDestroyed(Map::Vector2i gridPos, uint32_t ownerId) {}
    virtual void OnWallPickedUp(Map::Vector2i gridPos, uint32_t ownerId) {}
    virtual void OnWallInputDenied(uint32_t playerId) const {}

  private:
    std::unordered_map<Vector2i, DynamicWall, GridHash> m_walls;
};

} // namespace Map
