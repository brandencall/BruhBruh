#include "character_camera.hpp"

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

    m_hitSub = hitBus.Subscribe([this](const client::HitEvent &e) { OnHit(e); });
    m_deathSub = deathBus.Subscribe([this](const client::PlayerDiedEvent &e) { OnPlayerDied(e); });
    m_wallSub = wallBus.Subscribe([this](const client::WallPlacedEvent &e) { OnWallPlaced(e); });
}

void CharacterCamera::Update(float dt) {
    // Decay trauma
    m_shake = std::max(0.0f, m_shake - m_shakeDecay * dt);

    // Trauma curve
    float shakeAmount = m_shake * m_shake;

    // Random offsets
    float offsetX = ((float)GetRandomValue(-1000, 1000) / 1000.0f) * m_maxShakeOffset * shakeAmount;

    float offsetY = ((float)GetRandomValue(-1000, 1000) / 1000.0f) * m_maxShakeOffset * shakeAmount;

    // Apply shake
    m_camera.offset = {m_baseOffset.x + offsetX, m_baseOffset.y + offsetY};
}

void CharacterCamera::SetPosition(Vector2 position) { m_camera.target = position; }

const Camera2D *CharacterCamera::GetCamera() const { return &m_camera; }

void CharacterCamera::OnHit(const client::HitEvent &e) {
    if (e.victimId == e.localPlayerId)
        AddShake(0.5);
}

void CharacterCamera::OnPlayerDied(const client::PlayerDiedEvent &e) {
    if (e.data.victim.id == e.localPlayer.id)
        AddShake(2.5);
}

void CharacterCamera::OnWallPlaced(const client::WallPlacedEvent &e) {
    if (e.wallPlacerId == e.localPlayerId)
        AddShake(.75);
}

void CharacterCamera::AddShake(float amount) { m_shake = std::min(1.0f, m_shake + amount); }

} // namespace System
