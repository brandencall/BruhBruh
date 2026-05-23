#include "character_camera.hpp"
#include "raylib.h"

namespace System {

void CharacterCamera::Init(Client::EventBus<client::HitEvent> &hitBus,
                           Client::EventBus<client::PlayerDiedEvent> &deathBus,
                           Client::EventBus<client::WallPlacedEvent> &wallBus) {
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();
    m_baseOffset = {std::round(screenW / 2.0f), std::round(screenH / 2.0f)};

    m_camera.offset = m_baseOffset;
    m_camera.rotation = 0.0f;
    m_camera.zoom = 1.5f;

    m_damageShader = LoadShader(nullptr, "assets/shaders/screen_damage.fs");
    m_damageLoc = GetShaderLocation(m_damageShader, "damage");
    m_damageEffect = 0.0f;

    m_hitSub = hitBus.Subscribe([this](const client::HitEvent &e) { OnHit(e); });
    m_deathSub = deathBus.Subscribe([this](const client::PlayerDiedEvent &e) { OnPlayerDied(e); });
    m_wallSub = wallBus.Subscribe([this](const client::WallPlacedEvent &e) { OnWallPlaced(e); });
}

void CharacterCamera::Update(float dt) {
    m_shake = std::max(0.0f, m_shake - m_shakeDecay * dt);
    float shakeAmount = m_shake * m_shake;
    float offsetX = ((float)GetRandomValue(-1000, 1000) / 1000.0f) * m_maxShakeOffset * shakeAmount;
    float offsetY = ((float)GetRandomValue(-1000, 1000) / 1000.0f) * m_maxShakeOffset * shakeAmount;
    m_camera.offset = {m_baseOffset.x + offsetX, m_baseOffset.y + offsetY};

    m_damageEffect -= dt * 5.0f;
    m_damageEffect = Clamp(m_damageEffect, 0.0f, 1.0f);
}

void CharacterCamera::RenderDamageShader(RenderTexture2D texture) {
    BeginShaderMode(m_damageShader);

    SetShaderValue(m_damageShader, m_damageLoc, &m_damageEffect, SHADER_UNIFORM_FLOAT);

    DrawTextureRec(texture.texture, Rectangle{0, 0, (float)texture.texture.width, -(float)texture.texture.height},
                   Vector2{0, 0}, WHITE);

    EndShaderMode();
}

void CharacterCamera::SetPosition(Vector2 position) { m_camera.target = position; }

const Camera2D *CharacterCamera::GetCamera() const { return &m_camera; }

void CharacterCamera::OnHit(const client::HitEvent &e) {
    if (e.victimId == e.localPlayerId) {
        AddShake(0.3);
        m_damageEffect = 1.0f;
    }
}

void CharacterCamera::OnPlayerDied(const client::PlayerDiedEvent &e) {
    if (e.data.victim.id == e.localPlayer.id)
        AddShake(2.5);
}

void CharacterCamera::OnWallPlaced(const client::WallPlacedEvent &e) {
    constexpr float MAX_SHAKE_DISTANCE = 600.0f;
    constexpr float MAX_SHAKE = 0.75f;

    if (e.wallPlacerId == e.localPlayerId)
        AddShake(MAX_SHAKE);

    Vector2 wallPlacedPosition = Map::GridToWorld(e.gridPos);
    float distance = Vector2Distance(e.localPlayerPosition, wallPlacedPosition);

    if (distance > MAX_SHAKE_DISTANCE)
        return;

    float shakeAmount = 1.0f - (distance / MAX_SHAKE_DISTANCE);
    AddShake(shakeAmount * MAX_SHAKE);
}

void CharacterCamera::AddShake(float amount) { m_shake = std::min(1.0f, m_shake + amount); }

} // namespace System
