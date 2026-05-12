#include "client_bullet_system.hpp"
#include "raylib.h"
#include <array>
#include <cstdint>

namespace System {

void ClientBulletSystem::Load() {
    m_textures[Character::CharacterId::Tonts] = LoadTexture("assets/items/bottle_tonts_v2.png");
    m_textures[Character::CharacterId::Hodge] = LoadTexture("assets/items/meatball_hodges_v1.png");
    m_textures[Character::CharacterId::Raff] = LoadTexture("assets/items/steak_raff_v1.png");
    m_textures[Character::CharacterId::JJ] = LoadTexture("assets/items/needle_j_v1.png");

    for (const auto &tex : m_textures) {
        SetTextureFilter(tex.second, TEXTURE_FILTER_POINT);
    }
}

void ClientBulletSystem::Unload() {
    for (auto &[id, tex] : m_textures)
        UnloadTexture(tex);
}

int ClientBulletSystem::Spawn(const BulletSpawnDef &bulletDef) {

    Vector2 velocity = Vector2Scale(Vector2Normalize(bulletDef.direction), bulletDef.character.bullet.speed);
    component::Hitbox hitbox = {
        .circle = {.center = {bulletDef.position.x, bulletDef.position.y}, .radius = bulletDef.character.bullet.radius},
        .damage = bulletDef.character.bullet.damage,
    };
    state::ClientBulletState bullet{};
    bullet.id = -1;
    bullet.ownerId = bulletDef.ownerId;
    bullet.characterId = bulletDef.character.id;
    bullet.predId = bulletDef.predSequence;
    bullet.velocity = velocity;
    bullet.lifetime = bulletDef.character.bullet.lifetime;
    bullet.rotation = atan2f(velocity.y, velocity.x) * RAD2DEG;
    bullet.hitbox = hitbox;
    bullet.active = true;
    bullet.serverPosition = bulletDef.position;
    bullet.bulletTexScale = bulletDef.character.bullet.bulletTexScale;
    m_predictedBullets[bulletDef.predSequence] = bullet;
    return 0;
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

    for (auto &bullet : m_predictedBullets) {
        if (!bullet.second.active)
            continue;
        UpdateBulletKinematics(bullet.second, dt);
    }

    for (auto &fx : m_hitEffects)
        fx.timer -= dt;

    std::erase_if(m_hitEffects, [](const HitEffect &fx) { return fx.timer <= 0.0f; });
}

void ClientBulletSystem::Draw() {
    for (const auto &bullet : m_bullets) {
        if (!bullet.active)
            continue;
        Draw(bullet);
    }

    for (const auto &bullet : m_predictedBullets) {
        if (!bullet.second.active)
            continue;
        Draw(bullet.second);
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

    Character::CharacterDef def = Character::GetCharacterDef(bullet.characterId);
    float diameter = bullet.hitbox.circle.radius * 2.0f * def.bullet.bulletTexScale;

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
    Character::CharacterDef def = Character::GetCharacterDef(characterId);
    bullet.serverPosition = spawnPos;
    bullet.characterId = characterId;
    bullet.bulletTexScale = def.bullet.bulletTexScale;
}

void ClientBulletSystem::OnBulletDestroyed(int slot, Vector2 position) {
    m_bullets[slot].serverPosition = position;
    m_bullets[slot].hitbox.circle.center = position;
    m_bullets[slot].lingerTimer = 0.02f;
    m_hitEffects.push_back({position, 0.15f, 0.15f});
}

int ClientBulletSystem::SpawnFromServerEvent(const network::BulletSpawnPacket &bullet) {
    int slot = GetSlot(bullet.bulletId);
    if (slot < 0 || slot >= MAX_BULLETS)
        return -1;

    Character::CharacterDef character = Character::GetCharacterDef(bullet.characterId);
    InitBulletSlot(slot, bullet.bulletId, bullet.ownerId, bullet.position, bullet.velocity, character);

    return slot;
}

void ClientBulletSystem::ResolveLocalPredictedBullet(const network::BulletSpawnPacket &bullet, uint32_t ownerId) {
    auto it = m_predictedBullets.find(bullet.bulletPredSequence);
    if (it != m_predictedBullets.end() && it->second.ownerId == ownerId) {
        SpawnFromServerEvent(bullet);
        m_predictedBullets.erase(it);
    }
}

void ClientBulletSystem::AssignId(int slot, uint32_t id) {
    if (slot < 0 || slot >= MAX_BULLETS)
        return;
    if (!m_bullets[slot].active)
        return;
    m_bullets[slot].id = id;
}

} // namespace System
