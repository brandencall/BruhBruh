#include "ability_pickup_renderer.hpp"

namespace Render {

void AbilityPickupRenderer::Load() {
    m_textures[{state::PickupType::Effect, static_cast<uint8_t>(state::EffectType::SpeedBoost)}] =
        LoadTexture("assets/abilities/speed_boost.png");
    m_textures[{state::PickupType::Effect, static_cast<uint8_t>(state::EffectType::DamageBoost)}] =
        LoadTexture("assets/abilities/damage_boost.png");

    m_textures[{state::PickupType::Ability, static_cast<uint8_t>(state::AbilityType::SlowShot)}] =
        LoadTexture("assets/abilities/slow_shot.png");

    for (const auto &tex : m_textures) {
        SetTextureFilter(tex.second, TEXTURE_FILTER_POINT);
    }
}

void AbilityPickupRenderer::Unload() {
    for (auto &[id, tex] : m_textures)
        UnloadTexture(tex);
}

void AbilityPickupRenderer::Draw(std::vector<state::AbilityPickup> &pickups) {
    for (const auto &p : pickups) {
        auto it = m_textures.find({p.pickupType, p.typeId});
        if (it == m_textures.end())
            return;
        const Texture2D &tex = it->second;
        const Vector2 &center = p.collider.center;
        float diameter = p.collider.radius * 2.0f;
        Rectangle source = {0, 0, (float)tex.width, (float)tex.height};
        float scale = diameter / (float)std::max(tex.width, tex.height);
        Rectangle dest = {center.x, center.y, tex.width * scale, tex.height * scale};
        Vector2 origin = {tex.width * scale * 0.5f, tex.height * scale * 0.5f};

        DrawTexturePro(tex, source, dest, origin, 0, WHITE);
    }
}

} // namespace Render
