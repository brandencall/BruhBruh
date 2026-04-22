#pragma once

#include "../../shared/characters/character_types.hpp"
#include "../../shared/map/dynamic_walls/dynamic_wall.hpp"
#include "../../shared/map/dynamic_walls/wall_manager.hpp"
#include "raylib.h"
#include <unordered_map>

namespace Render {

class WallRenderer {
  public:
    void Load();
    void Unload();
    void Draw(const std::unordered_map<Map::Vector2i, Map::DynamicWall, Map::GridHash> &walls);

  private:
    std::unordered_map<Character::CharacterId, Texture2D> m_textures;
};

} // namespace Render
