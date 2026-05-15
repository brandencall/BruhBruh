#pragma once

#include "../../shared/characters/character_types.hpp"
#include "../../shared/map/dynamic_walls/dynamic_wall.hpp"
#include "../../shared/map/dynamic_walls/wall_manager.hpp"
#include "../../shared/map/grid.hpp"
#include "raylib.h"
#include <unordered_map>

namespace Render {

struct DyingWall {
    Map::Vector2i gridPos;
    Character::CharacterId ownerCharacter;
    float deathTime;
};

class WallRenderer {
  public:
    void Load();
    void Unload();
    void Draw(const std::unordered_map<Map::Vector2i, Map::DynamicWall, Map::GridHash> &walls);
    void DrawWalls(const std::unordered_map<Map::Vector2i, Map::DynamicWall, Map::GridHash> &walls);
    void DrawDyingWalls();
    void AddDyingWall(const DyingWall &wall);
    void AddDyingWall(const Map::Vector2i &gridPos, const Character::CharacterId ownerCharacter);

  private:
    std::unordered_map<Character::CharacterId, Texture2D> m_textures;
    // std::vector<DyingWall> m_dyingWalls;
    std::unordered_map<Map::Vector2i, DyingWall, Map::GridHash> m_dyingWalls;
};

} // namespace Render
