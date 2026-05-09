#include "client_bullet_system.hpp"
#include "raylib.h"
#include <array>
#include <cstdint>

namespace System {

void ClientBulletSystem::Load() {
    m_textures[Character::CharacterId::Tonts] = LoadTexture("assets/items/bottle_tonts_v2.png");
    m_textures[Character::CharacterId::Hodge] = LoadTexture("assets/items/meatball_hodges_v1.png");
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

        if (bullet.lingerTimer > 0.0f) {
            bullet.lingerTimer -= dt;
            if (bullet.lingerTimer <= 0.0f)
                bullet.active = false;
            continue; // don't move it
        }

        UpdateBulletKinematics(bullet, dt);

        if (bullet.lifetime <= 0.0f) {
            bullet.active = false;
            continue;
        }
    }

    for (auto &fx : m_hitEffects)
        fx.timer -= dt;

    std::erase_if(m_hitEffects, [](const HitEffect &fx) { return fx.timer <= 0.0f; });
}

void ClientBulletSystem::Draw(const std::array<state::ClientBulletState, MAX_BULLETS> &bullets) {
    for (const auto &bullet : bullets) {
        if (!bullet.active)
            continue;
        Draw(bullet);
    }

    for (const auto &fx : m_hitEffects) {
        float t = 1.0f - (fx.timer / fx.maxTimer);
        float radius = 8.0f * t;
        uint8_t alpha = (uint8_t)(255 * (1.0f - t));
        DrawCircleV(fx.position, radius, {255, 200, 50, alpha});
    }
}

void ClientBulletSystem::Draw(const state::ClientBulletState &bullet) {
    auto it = m_textures.find(bullet.characterId);
    if (it == m_textures.end())
        return;
    const Texture2D &tex = it->second;
    const Vector2 &center = bullet.hitbox.circle.center;

    // texture renders at 125% of hitbox radius
    constexpr float kVisualScale = 1.25f;
    float diameter = bullet.hitbox.circle.radius * 2.0f * kVisualScale;

    Rectangle source = {0, 0, (float)tex.width, (float)tex.height};
    float scale = diameter / (float)std::max(tex.width, tex.height);
    Rectangle dest = {center.x, center.y, tex.width * scale, tex.height * scale};
    Vector2 origin = {tex.width * scale * 0.5f, tex.height * scale * 0.5f};

    DrawTexturePro(tex, source, dest, origin, bullet.rotation, WHITE);
    // debug hitbox
    // DrawCircleV(center, bullet.hitbox.circle.radius, {255, 0, 0, 80});
    // DrawCircleLinesV(center, bullet.hitbox.circle.radius, YELLOW);
}

void ClientBulletSystem::OnSpawn(state::ClientBulletState &bullet, Vector2 spawnPos,
                                 Character::CharacterId characterId) {
    bullet.serverPosition = spawnPos;
    bullet.characterId = characterId;
}

void ClientBulletSystem::OnBulletDestroyed(int slot, Vector2 position) {
    m_bullets[slot].serverPosition = position;
    m_bullets[slot].hitbox.circle.center = position;
    m_bullets[slot].lingerTimer = 0.02f;
    m_hitEffects.push_back({position, 0.15f, 0.15f});
}

int ClientBulletSystem::SpawnFromServerEvent(uint32_t serverId, uint32_t ownerId, Vector2 position, Vector2 velocity,
                                             const Character::CharacterDef &character) {
    int slot = GetSlot(serverId);
    if (slot < 0 || slot >= MAX_BULLETS)
        return -1;

    InitBulletSlot(slot, serverId, ownerId, position, velocity, character);

    return slot;
}

void ClientBulletSystem::AssignId(int slot, uint32_t id) {
    if (slot < 0 || slot >= MAX_BULLETS)
        return;
    if (!m_bullets[slot].active)
        return;
    m_bullets[slot].id = id;
}

} // namespace System
