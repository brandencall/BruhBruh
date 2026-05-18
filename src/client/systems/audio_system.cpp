#include "audio_system.hpp"
#include "raylib.h"
#include "raymath.h"

namespace System {

void AudioSystem::InitGamePlay(Client::EventBus<client::HitEvent> &hitBus,
                               Client::EventBus<client::PlayerDiedEvent> &deathBus,
                               Client::EventBus<client::WallPlacedEvent> &wallPlacedBus,
                               Client::EventBus<client::WallPickedUpEvent> &wallPickedUpBus) {
    if (!IsAudioDeviceReady())
        return;

    m_hitmarkerSound = LoadSound("assets/sounds/hitmarker.wav");
    m_deathSound = LoadSound("assets/sounds/dramatic_death.wav");
    m_killRewardSound = LoadSound("assets/sounds/kill_reward.wav");
    m_wallPlacedConcreteSound = LoadSound("assets/sounds/wall_placed_concrete.wav");
    m_wallPlacedKickDrumSound = LoadSound("assets/sounds/wall_placed_kick_drum.wav");
    m_wallPickedUpSound = LoadSound("assets/sounds/wall_whoosh.wav");

    for (int i = 0; i < HITMARKER_POOL_SIZE; ++i) {
        m_hitmarkerAliases[i] = LoadSoundAlias(m_hitmarkerSound);
    }

    for (int i = 0; i < WALL_PLACEMENT_POOL_SIZE; ++i) {
        m_wallPlacementConcreteAliases[i] = LoadSoundAlias(m_wallPlacedConcreteSound);
        m_wallPlacementKickDrumAliases[i] = LoadSoundAlias(m_wallPlacedKickDrumSound);
    }

    for (int i = 0; i < WALL_PICKEDUP_POOL_SIZE; ++i) {
        m_wallPickedUpAliases[i] = LoadSoundAlias(m_wallPickedUpSound);
    }

    m_gameplaySubs.clear();
    m_gameplaySubs.emplace_back(hitBus.Subscribe([this](const client::HitEvent &e) { OnHit(e); }));
    m_gameplaySubs.emplace_back(deathBus.Subscribe([this](const client::PlayerDiedEvent &e) { OnPlayerDied(e); }));
    m_gameplaySubs.emplace_back(wallPlacedBus.Subscribe([this](const client::WallPlacedEvent &e) { OnWallPlaced(e); }));
    m_gameplaySubs.emplace_back(
        wallPickedUpBus.Subscribe([this](const client::WallPickedUpEvent &e) { OnWallPickedUp(e); }));
}

void AudioSystem::UnloadGamePlay() {
    for (Sound &s : m_hitmarkerAliases) {
        SafeUnloadAlias(s);
    }
    for (Sound &s : m_wallPlacementConcreteAliases) {
        SafeUnloadAlias(s);
    }
    for (Sound &s : m_wallPlacementKickDrumAliases) {
        SafeUnloadAlias(s);
    }
    for (Sound &s : m_wallPickedUpAliases) {
        SafeUnloadAlias(s);
    }

    SafeUnload(m_hitmarkerSound);
    SafeUnload(m_deathSound);
    SafeUnload(m_killRewardSound);
    SafeUnload(m_wallPlacedConcreteSound);
    SafeUnload(m_wallPlacedKickDrumSound);

    m_gameplaySubs.clear();
}

void AudioSystem::Unload() { UnloadGamePlay(); }

void AudioSystem::SafeUnload(Sound &s) {
    if (s.stream.buffer != nullptr) {
        UnloadSound(s);
        s = {};
    }
}

void AudioSystem::SafeUnloadAlias(Sound &s) {
    if (s.stream.buffer != nullptr) {
        UnloadSoundAlias(s);
        s = {};
    }
}

void AudioSystem::SetMasterVolume(float volume) { m_masterVolume = volume; }

void AudioSystem::SetMusicVolume(float volume) { m_musicVolume = volume; }

void AudioSystem::SetEffectsVolume(float volume) { m_effectsVolume = volume; }

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

    PlaySpatialSound2D(m_deathSound, e.data.victim.position, 800.0, e.localPlayer.position, SoundCategory::Effects);
}

void AudioSystem::OnWallPlaced(const client::WallPlacedEvent &event) {
    float concreteSoundVolume = 0.2f;
    Sound &concreteAlias = m_wallPlacementConcreteAliases[m_wallPlacementIndex];
    Sound &kickDrumAlias = m_wallPlacementKickDrumAliases[m_wallPlacementIndex];
    m_wallPlacementIndex = (m_wallPlacementIndex + 1) % WALL_PLACEMENT_POOL_SIZE;

    float pitch = 0.95f + (GetRandomValue(0, 10) / 100.0f);

    SetSoundPitch(concreteAlias, pitch);
    SetSoundPitch(kickDrumAlias, pitch);

    Vector2 wallPlacedPosition = Map::GridToWorld(event.gridPos);
    PlaySpatialSound2D(concreteAlias, wallPlacedPosition, 800.0, event.localPlayerPosition, SoundCategory::Effects,
                       concreteSoundVolume);
    PlaySpatialSound2D(kickDrumAlias, wallPlacedPosition, 800.0, event.localPlayerPosition, SoundCategory::Effects);
}

void AudioSystem::OnWallPickedUp(const client::WallPickedUpEvent &event) {
    float pickUpSoundVolume = 0.2f;
    Sound &alias = m_wallPickedUpAliases[m_wallPickedUpIndex];
    m_wallPickedUpIndex = (m_wallPickedUpIndex + 1) % WALL_PICKEDUP_POOL_SIZE;
    float pitch = 0.95f + (GetRandomValue(0, 10) / 100.0f);

    SetSoundPitch(alias, pitch);

    Vector2 wallPickedUpPosition = Map::GridToWorld(event.gridPos);
    PlaySpatialSound2D(alias, wallPickedUpPosition, 800.0, event.localPlayerPosition, SoundCategory::Effects,
                       pickUpSoundVolume);
}

void AudioSystem::PlayHitmarker() {
    Sound &alias = m_hitmarkerAliases[m_hitmarkerIndex];
    m_hitmarkerIndex = (m_hitmarkerIndex + 1) % HITMARKER_POOL_SIZE;

    float pitch = 0.95f + (GetRandomValue(0, 10) / 100.0f);

    SetSoundPitch(alias, pitch);
    SetSoundVolume(alias, 0.3f);

    Play(alias, SoundCategory::Effects);
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

void AudioSystem::PlaySpatialSound2D(Sound sound, Vector2 soundPos, float maxRange, Vector2 localPlayerPos,
                                     SoundCategory category, float soundVolume) {
    float volume = GetSpatialVolume2D(soundPos, maxRange, localPlayerPos) * soundVolume;
    if (volume <= 0.0f)
        return;

    float pan = GetSpatialPan2D(soundPos, localPlayerPos);

    SetSoundPan(sound, (pan + 1.0f) / 2.0f);
    Play(sound, category, volume);
}

void AudioSystem::Play(Sound sound, SoundCategory category, float soundVolume) {
    float volume = soundVolume * m_masterVolume;
    if (category == SoundCategory::Music) {
        volume *= m_musicVolume;
    } else if (category == SoundCategory::Effects) {
        volume *= m_effectsVolume;
    }
    SetSoundVolume(sound, volume);
    PlaySound(sound);
}

} // namespace System
