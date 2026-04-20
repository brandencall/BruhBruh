#pragma once
#include "../../shared/map/grid.hpp"
#include "../../shared/map/tiles/tilemap_loader.hpp"
#include "raylib.h"
#include "tileset.hpp"

namespace Render {

class TilemapRenderer {
  public:
    void Load(const Map::TilesetDef &def) {
        m_tileset.Load(def);
        SetTextureFilter(m_tileset.GetTexture(), TEXTURE_FILTER_POINT);
    }
    void Unload() { m_tileset.Unload(); }

    void Draw(const Map::TileMap &map) const {
        const float S = (float)Map::GRID_CELL_SIZE;

        for (int y = 0; y < map.height; y++) {
            for (int x = 0; x < map.width; x++) {
                int tileId = static_cast<int>(map.At(x, y));
                Rectangle src = m_tileset.GetSourceRect(tileId);
                Rectangle dest = {(float)(x * Map::GRID_CELL_SIZE), (float)(y * Map::GRID_CELL_SIZE),
                                  (float)Map::GRID_CELL_SIZE, (float)Map::GRID_CELL_SIZE};
                DrawTexturePro(m_tileset.GetTexture(), src, dest, {0, 0}, 0.0f, WHITE);
            }
        }
    }

  private:
    Tileset m_tileset;
};

} // namespace Render
