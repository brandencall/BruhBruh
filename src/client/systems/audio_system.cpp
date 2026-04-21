#include "audio_system.hpp"
#include "raylib.h"
#include "raymath.h"

namespace System {

AudioSystem::AudioSystem(state::PlayerState &localPlayer) : m_localPlayer(localPlayer) {}

void AudioSystem::Init(Client::EventBus<client::HitEvent> &hitBus, Client::EventBus<event::PlayerDiedEvent> &deathBus) {
    m_hitmarkerSound = LoadSound("assets/sounds/hitmarker.wav");
    SetSoundVolume(m_hitmarkerSound, 0.3);
    m_deathSound = LoadSound("assets/sounds/dramatic_death.wav");

    for (int i = 0; i < HITMARKER_POOL_SIZE; ++i) {
        m_hitmarkerAliases[i] = LoadSoundAlias(m_hitmarkerSound);
    }
    hitBus.Subscribe([this](const client::HitEvent &e) { OnHit(e); });
    deathBus.Subscribe([this](const event::PlayerDiedEvent &e) { OnPlayerDied(e); });
}

void AudioSystem::Unload() {
    for (int i = 0; i < HITMARKER_POOL_SIZE; i++) {
        UnloadSound(m_hitmarkerAliases[i]);
    }

    UnloadSound(m_hitmarkerSound);
    CloseAudioDevice();
}

void AudioSystem::OnHit(const client::HitEvent &e) {
    if (e.attackerId == m_localPlayer.id) {
        PlayHitmarker();
    }

    // optional: character-specific hit sounds
    // PlayCharacterHitSound(e.attackerCharacter);
}

void AudioSystem::OnPlayerDied(const event::PlayerDiedEvent &e) {
    if (e.victim.id == m_localPlayer.id)
        return;

    // Play a reward sound
    // if (e.killer.id == m_localPlayer.id)

    PlaySpatialSound2D(m_deathSound, e.victim.position, 800.0);
}

void AudioSystem::PlayHitmarker() {
    Sound &alias = m_hitmarkerAliases[m_hitmarkerIndex];
    m_hitmarkerIndex = (m_hitmarkerIndex + 1) % HITMARKER_POOL_SIZE;

    float pitch = 0.95f + (GetRandomValue(0, 10) / 100.0f);

    SetSoundPitch(alias, pitch);
    SetSoundVolume(alias, 0.3f);

    PlaySound(alias);
}

float AudioSystem::GetSpatialVolume2D(Vector2 soundPos, float maxRange) {
    float dist = Vector2Distance(m_localPlayer.position, soundPos);
    if (dist >= maxRange)
        return 0.0f;
    return 1.0f - (dist / maxRange);
}

float AudioSystem::GetSpatialPan2D(Vector2 soundPos) {
    Vector2 toSound = Vector2Normalize(Vector2Subtract(soundPos, m_localPlayer.position));
    // -1.0 (left) to 1.0 (right)
    return toSound.x;
}

void AudioSystem::PlaySpatialSound2D(Sound sound, Vector2 soundPos, float maxRange) {
    float volume = GetSpatialVolume2D(soundPos, maxRange);
    if (volume <= 0.0f)
        return;

    float pan = GetSpatialPan2D(soundPos);

    SetSoundVolume(sound, volume);
    SetSoundPan(sound, (pan + 1.0f) / 2.0f);
    PlaySound(sound);
}

void AudioSystem::Play(Sound sound) {}

} // namespace System
