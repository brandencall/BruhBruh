#include "wall_renderer.hpp"
#include <unordered_map>

namespace Render {

void WallRenderer::Load() {
    m_textures[Character::CharacterId::Tonts] = LoadTexture("assets/tmp_wall.png");
    m_textures[Character::CharacterId::Raff] = LoadTexture("assets/tmp_wall.png");
    m_textures[Character::CharacterId::Hodge] = LoadTexture("assets/tmp_wall.png");
    m_textures[Character::CharacterId::JJ] = LoadTexture("assets/tmp_wall.png");
}

void WallRenderer::Unload() {
    for (auto &[id, tex] : m_textures)
        UnloadTexture(tex);
}

void WallRenderer::Draw(const std::unordered_map<Map::Vector2i, Map::DynamicWall, Map::GridHash> &walls) {
    for (const auto &[gridPos, wall] : walls) {
        if (!wall.active)
            continue;

        auto it = m_textures.find(wall.ownerCharacter);
        if (it == m_textures.end())
            continue;

        Texture2D &tex = it->second;

        float x = gridPos.x * Map::GRID_CELL_SIZE;
        float y = gridPos.y * Map::GRID_CELL_SIZE;

        Rectangle src = {0, 0, (float)tex.width, (float)tex.height};
        Rectangle dst = {x, y, Map::GRID_CELL_SIZE, Map::GRID_CELL_SIZE};

        DrawTexturePro(tex, src, dst, {0, 0}, 0.0f, WHITE);
    }
}

} // namespace Render
