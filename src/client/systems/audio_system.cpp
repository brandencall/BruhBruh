#include "audio_system.hpp"
#include "raylib.h"
#include "raymath.h"

namespace System {

void AudioSystem::Init(Client::EventBus<client::HitEvent> &hitBus,
                       Client::EventBus<client::PlayerDiedEvent> &deathBus) {
    m_hitmarkerSound = LoadSound("assets/sounds/hitmarker.wav");
    SetSoundVolume(m_hitmarkerSound, 0.3);
    m_deathSound = LoadSound("assets/sounds/dramatic_death.wav");
    m_killRewardSound = LoadSound("assets/sounds/kill_reward.wav");

    for (int i = 0; i < HITMARKER_POOL_SIZE; ++i) {
        m_hitmarkerAliases[i] = LoadSoundAlias(m_hitmarkerSound);
    }
    m_hitSub = hitBus.Subscribe([this](const client::HitEvent &e) { OnHit(e); });
    m_deathSub = deathBus.Subscribe([this](const client::PlayerDiedEvent &e) { OnPlayerDied(e); });
}

void AudioSystem::Unload() {
    UnloadSound(m_hitmarkerSound);
    UnloadSound(m_deathSound);
    UnloadSound(m_killRewardSound);
}

void AudioSystem::OnHit(const client::HitEvent &e) {
    if (e.attackerId == e.localPlayerId) {
        PlayHitmarker();
    }
}

void AudioSystem::OnPlayerDied(const client::PlayerDiedEvent &e) {
    if (e.data.victim.id == e.localPlayer.id)
        return;

    if (e.data.killer.id == e.localPlayer.id)
        PlaySound(m_killRewardSound);

    PlaySpatialSound2D(m_deathSound, e.data.victim.position, 800.0, e.localPlayer.position);
}

void AudioSystem::PlayHitmarker() {
    Sound &alias = m_hitmarkerAliases[m_hitmarkerIndex];
    m_hitmarkerIndex = (m_hitmarkerIndex + 1) % HITMARKER_POOL_SIZE;

    float pitch = 0.95f + (GetRandomValue(0, 10) / 100.0f);

    SetSoundPitch(alias, pitch);
    SetSoundVolume(alias, 0.3f);

    PlaySound(alias);
}

float AudioSystem::GetSpatialVolume2D(Vector2 soundPos, float maxRange, Vector2 localPlayerPos) {
    float dist = Vector2Distance(localPlayerPos, soundPos);
    if (dist >= maxRange)
        return 0.0f;
    return 1.0f - (dist / maxRange);
}

float AudioSystem::GetSpatialPan2D(Vector2 soundPos, Vector2 localPlayerPos) {
    Vector2 toSound = Vector2Normalize(Vector2Subtract(soundPos, localPlayerPos));
    // -1.0 (left) to 1.0 (right)
    return toSound.x;
}

void AudioSystem::PlaySpatialSound2D(Sound sound, Vector2 soundPos, float maxRange, Vector2 localPlayerPos) {
    float volume = GetSpatialVolume2D(soundPos, maxRange, localPlayerPos);
    if (volume <= 0.0f)
        return;

    float pan = GetSpatialPan2D(soundPos, localPlayerPos);

    SetSoundVolume(sound, volume);
    SetSoundPan(sound, (pan + 1.0f) / 2.0f);
    PlaySound(sound);
}

void AudioSystem::Play(Sound sound) {}

} // namespace System
