#include "client_bullet_system.hpp"
#include "raylib.h"

namespace System {

void ClientBulletSystem::Load() {
    m_textures[Character::CharacterId::Tonts] = LoadTexture("assets/characters/tmp/bottle_tonts.png");
    m_textures[Character::CharacterId::Raff] = LoadTexture("assets/characters/tmp/bottle_tonts.png");
    m_textures[Character::CharacterId::Hodge] = LoadTexture("assets/characters/tmp/bottle_tonts.png");
    m_textures[Character::CharacterId::JJ] = LoadTexture("assets/characters/tmp/bottle_tonts.png");
}

void ClientBulletSystem::Unload() {
    for (auto &[id, tex] : m_textures)
        UnloadTexture(tex);
}

void ClientBulletSystem::Update(float dt) {
    for (auto &bullet : m_bullets) {
        if (!bullet.active)
            continue;

        bullet.hitbox.circle.center.x += bullet.velocity.x * dt;
        bullet.hitbox.circle.center.y += bullet.velocity.y * dt;
        bullet.lifetime -= dt;

        if (bullet.lifetime <= 0.0f) {
            bullet.active = false;
            continue;
        }
    }
}

void ClientBulletSystem::Draw(const state::ClientBulletState &bullet) {
    auto it = m_textures.find(bullet.characterId);
    if (it == m_textures.end())
        return;
    // Draw centered on player position
    DrawTextureV(it->second, bullet.hitbox.circle.center, WHITE);
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
