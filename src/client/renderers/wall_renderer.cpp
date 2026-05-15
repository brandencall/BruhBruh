#include "wall_renderer.hpp"
#include "raylib.h"
#include <unordered_map>

namespace Render {

void WallRenderer::Load() {
    m_textures[Character::CharacterId::Tonts] = LoadTexture("assets/walls/tonts_wall_v1.png");
    m_textures[Character::CharacterId::Raff] = LoadTexture("assets/walls/raff_wall.png");
    m_textures[Character::CharacterId::Hodge] = LoadTexture("assets/walls/hodges_wall_v1.png");
    m_textures[Character::CharacterId::JJ] = LoadTexture("assets/walls/jonty_wall_v1.png");

    for (const auto &tex : m_textures) {
        SetTextureFilter(tex.second, TEXTURE_FILTER_POINT);
    }
}

void WallRenderer::Unload() {
    for (auto &[id, tex] : m_textures)
        UnloadTexture(tex);
}

void WallRenderer::Draw(const std::unordered_map<Map::Vector2i, Map::DynamicWall, Map::GridHash> &walls) {
    DrawWalls(walls);
    DrawDyingWalls();
}

void WallRenderer::DrawWalls(const std::unordered_map<Map::Vector2i, Map::DynamicWall, Map::GridHash> &walls) {
    for (const auto &[gridPos, wall] : walls) {
        if (!wall.active)
            continue;
        auto it = m_textures.find(wall.ownerCharacter);
        if (it == m_textures.end())
            continue;
        Texture2D &tex = it->second;

        float x = floorf(gridPos.x * Map::GRID_CELL_SIZE);
        float y = floorf(gridPos.y * Map::GRID_CELL_SIZE);

        // Bounce animation
        const float animDuration = 0.3f;
        float elapsed = (float)GetTime() - wall.spawnTime;
        float scale = 1.0f;

        if (wall.spawnTime > 0.0f && elapsed < animDuration) {
            float t = elapsed / animDuration;
            scale = 1.0f + 0.35f * sinf(t * PI) * (1.0f - t);
        }

        float size = Map::GRID_CELL_SIZE * scale;
        float offset = (Map::GRID_CELL_SIZE - size) * 0.5f; // keep centered on tile

        Rectangle src = {0, 0, (float)tex.width, (float)tex.height};
        Rectangle dst = {x + offset, y + offset, size, size};
        DrawTexturePro(tex, src, dst, {0, 0}, 0.0f, WHITE);
    }
}

void WallRenderer::DrawDyingWalls() {
    const float animDuration = 0.25f;
    float now = (float)GetTime();

    std::erase_if(m_dyingWalls, [&](const auto &pair) { return (now - pair.second.deathTime) >= animDuration; });

    for (const auto &[gridPos, dw] : m_dyingWalls) {
        auto it = m_textures.find(dw.ownerCharacter);
        if (it == m_textures.end())
            continue;
        Texture2D &tex = it->second;

        float elapsed = now - dw.deathTime;
        float t = elapsed / animDuration;

        // Shrinks and fades out
        float scale = 1.0f - (t * t); // ease-in shrink
        float alpha = 1.0f - t;

        float x = floorf(dw.gridPos.x * Map::GRID_CELL_SIZE);
        float y = floorf(dw.gridPos.y * Map::GRID_CELL_SIZE);
        float size = Map::GRID_CELL_SIZE * scale;
        float offset = (Map::GRID_CELL_SIZE - size) * 0.5f;

        Rectangle src = {0, 0, (float)tex.width, (float)tex.height};
        Rectangle dst = {x + offset, y + offset, size, size};
        DrawTexturePro(tex, src, dst, {0, 0}, 0.0f, Fade(WHITE, alpha));
    }
}

void WallRenderer::AddDyingWall(const Map::Vector2i &gridPos, Character::CharacterId ownerCharacter) {
    m_dyingWalls[gridPos] =
        DyingWall{.gridPos = gridPos, .ownerCharacter = ownerCharacter, .deathTime = (float)GetTime()};
}

} // namespace Render
