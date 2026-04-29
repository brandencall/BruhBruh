#include "client_bullet_system.hpp"
#include "raylib.h"
#include <array>

namespace System {

void ClientBulletSystem::Load() {
    m_textures[Character::CharacterId::Tonts] = LoadTexture("assets/items/bottle_tonts.png");
    m_textures[Character::CharacterId::Hodge] = LoadTexture("assets/items/meatball_hodges.png");
    m_textures[Character::CharacterId::Raff] = LoadTexture("assets/items/steak_raff.png");
    m_textures[Character::CharacterId::JJ] = LoadTexture("assets/items/needle_j.png");

    for (const auto &tex : m_textures) {
        SetTextureFilter(tex.second, TEXTURE_FILTER_POINT);
    }
}

void ClientBulletSystem::Unload() {
    for (auto &[id, tex] : m_textures)
        UnloadTexture(tex);
}

void ClientBulletSystem::Update(float dt) {
    for (auto &bullet : m_bullets) {
        if (!bullet.active)
            continue;

        UpdateBulletKinematics(bullet, dt);

        if (bullet.lifetime <= 0.0f) {
            bullet.active = false;
            continue;
        }
    }
}

void ClientBulletSystem::Draw(const std::array<state::ClientBulletState, MAX_BULLETS> &bullets) {
    for (const auto &bullet : bullets) {
        if (!bullet.active)
            continue;
        Draw(bullet);
    }
}

void ClientBulletSystem::Draw(const state::ClientBulletState &bullet) {
    auto it = m_textures.find(bullet.characterId);
    if (it == m_textures.end())
        return;

    const Texture2D &tex = it->second;
    const Vector2 &center = bullet.hitbox.circle.center;

    Rectangle source = {0, 0, (float)tex.width, (float)tex.height};
    Rectangle dest = {center.x, center.y, (float)tex.width, (float)tex.height};
    Vector2 origin = {tex.width * 0.5f, tex.height * 0.5f};

    DrawTexturePro(tex, source, dest, origin, bullet.rotation, WHITE);

    // debug hitbox
    // DrawCircleV(center, bullet.hitbox.circle.radius, {255, 0, 0, 80});
    // DrawCircleLinesV(center, bullet.hitbox.circle.radius, RED);
}

void ClientBulletSystem::OnSpawn(state::ClientBulletState &bullet, Vector2 spawnPos,
                                 Character::CharacterId characterId) {
    bullet.serverPosition = spawnPos;
    bullet.characterId = characterId;
}

void ClientBulletSystem::AssignId(int slot, uint32_t id) {
    if (slot < 0 || slot >= MAX_BULLETS)
        return;
    if (!m_bullets[slot].active)
        return;
    m_bullets[slot].id = id;
}

} // namespace System
