#pragma once
#include "../../shared/map/tiles/tileset_def.hpp"
#include "raylib.h"

namespace Render {

class Tileset {
  public:
    void Load(const Map::TilesetDef &def) {
        m_texture = LoadTexture(def.texturePath.c_str());
        SetTextureFilter(m_texture, TEXTURE_FILTER_POINT);
        m_tileSize = def.tileSize;
        m_columns = def.columns;
    }

    void Unload() { UnloadTexture(m_texture); }

    Rectangle GetSourceRect(int tileId) const {
        int col = tileId % m_columns;
        int row = tileId / m_columns;
        return {(float)(col * m_tileSize), (float)(row * m_tileSize), (float)m_tileSize, (float)m_tileSize};
    }

    Texture2D GetTexture() const { return m_texture; }
    int GetTileSize() const { return m_tileSize; }

  private:
    Texture2D m_texture{};
    int m_tileSize = 16;
    int m_columns = 1;
};

} // namespace Render
