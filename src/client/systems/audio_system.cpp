#include "audio_system.hpp"
#include "../../shared/characters/character_roster.hpp"
#include "raylib.h"
#include "raymath.h"
#include <filesystem>

namespace System {

void AudioSystem::Load() {
    if (!IsAudioDeviceReady())
        return;

    std::string path = ConfigPath();
    FILE *f = std::fopen(path.c_str(), "r");
    if (!f)
        return; // first launch, defaults are fine

    char line[128];
    while (std::fgets(line, sizeof(line), f)) {
        float v;
        if (std::sscanf(line, "music_volume=%f", &v) == 1)
            SetMusicVolume(v);
        if (std::sscanf(line, "effects_volume=%f", &v) == 1)
            SetEffectsVolume(v);
        if (std::sscanf(line, "master_volume=%f", &v) == 1)
            SetMasterVolume(v);
    }
    std::fclose(f);
}

void AudioSystem::Save() {
    std::string path = ConfigPath();
    // Make sure the directory exists first
    std::filesystem::create_directories(std::filesystem::path(path).parent_path());

    FILE *f = std::fopen(path.c_str(), "w");
    if (!f)
        return;
    std::fprintf(f, "[audio]\n");
    std::fprintf(f, "music_volume=%.2f\n", m_musicVolume);
    std::fprintf(f, "effects_volume=%.2f\n", m_effectsVolume);
    std::fprintf(f, "master_volume=%.2f\n", m_masterVolume);
    std::fclose(f);
}

std::string AudioSystem::ConfigPath() {
#ifdef _WIN32
    // e.g. C:\Users\You\AppData\Roaming\BruhBruh\config.ini
    const char *appdata = std::getenv("APPDATA");
    return std::string(appdata) + "\\BruhBruh\\config.ini";
#elif __APPLE__
    const char *home = std::getenv("HOME");
    return std::string(home) + "/Library/Application Support/BruhBruh/config.ini";
#else // Linux
    const char *home = std::getenv("HOME");
    return std::string(home) + "/.config/BruhBruh/config.ini";
#endif
}

void AudioSystem::LoadMatchSounds() {
    if (!IsAudioDeviceReady())
        return;

    SafeLoad(m_countdownSound, "assets/sounds/countdown_beep.wav");
}

void AudioSystem::InitLobby(Client::EventBus<client::GameStartingEvent> &startingBus) {
    if (!IsAudioDeviceReady())
        return;

    LoadMatchSounds();
    m_countdownSub = startingBus.Subscribe([this](const client::GameStartingEvent &e) { OnCountdown(e); });
}

void AudioSystem::InitGamePlay(Client::EventHub &events) {
    if (!IsAudioDeviceReady())
        return;

    LoadMatchSounds();
    m_goBellSound = LoadSound("assets/sounds/go_bell_sound.wav");
    m_goBellLoaded = true;

    m_hitmarkerSound = LoadSound("assets/sounds/hitmarker.wav");
    m_deathSound = LoadSound("assets/sounds/dramatic_death.wav");
    m_killRewardSound = LoadSound("assets/sounds/kill_reward.wav");
    m_hurtGruntSound = LoadSound("assets/sounds/hurt_grunt.wav");

    m_wallPlacedConcreteSound = LoadSound("assets/sounds/wall_placed_concrete.wav");
    m_wallPlacedKickDrumSound = LoadSound("assets/sounds/wall_placed_kick_drum.wav");
    m_wallPickedUpSound = LoadSound("assets/sounds/wall_whoosh.wav");
    m_wallInputDeniedSound = LoadSound("assets/sounds/error.wav");

    for (int i = 0; i < HITMARKER_POOL_SIZE; ++i) {
        m_hitmarkerAliases[i] = LoadSoundAlias(m_hitmarkerSound);
    }

    for (int i = 0; i < HURT_GRUNT_POOL_SIZE; ++i) {
        m_hurtGruntAliases[i] = LoadSoundAlias(m_hurtGruntSound);
    }

    for (int i = 0; i < WALL_PLACEMENT_POOL_SIZE; ++i) {
        m_wallPlacementConcreteAliases[i] = LoadSoundAlias(m_wallPlacedConcreteSound);
        m_wallPlacementKickDrumAliases[i] = LoadSoundAlias(m_wallPlacedKickDrumSound);
    }

    for (int i = 0; i < WALL_PICKEDUP_POOL_SIZE; ++i) {
        m_wallPickedUpAliases[i] = LoadSoundAlias(m_wallPickedUpSound);
    }

    for (int i = 0; i < WALL_INPUT_DENIED_POOL_SIZE; ++i) {
        m_wallInputDeniedAliases[i] = LoadSoundAlias(m_wallInputDeniedSound);
    }

    m_gameplaySubs.clear();
    m_gameplaySubs.emplace_back(events.onHit.Subscribe([this](const client::HitEvent &e) { OnHit(e); }));
    m_gameplaySubs.emplace_back(
        events.bulletDestroyed.Subscribe([this](const client::BulletDestroyedEvent &e) { OnBulletDestroyed(e); }));
    m_gameplaySubs.emplace_back(
        events.playerDied.Subscribe([this](const client::PlayerDiedEvent &e) { OnPlayerDied(e); }));
    m_gameplaySubs.emplace_back(
        events.onWallPlaced.Subscribe([this](const client::WallPlacedEvent &e) { OnWallPlaced(e); }));
    m_gameplaySubs.emplace_back(
        events.onWallPickedUp.Subscribe([this](const client::WallPickedUpEvent &e) { OnWallPickedUp(e); }));
    m_gameplaySubs.emplace_back(
        events.onWallInputDenied.Subscribe([this](const event::WallInputDeniedEvent &e) { OnWallInputDenied(e); }));
    m_gameplaySubs.emplace_back(
        events.onGameStarting.Subscribe([this](const client::GameStartingEvent &e) { OnCountdown(e); }));
}

void AudioSystem::InitCharacterBulletSounds(std::array<state::PlayerState, MAX_PLAYERS> &players) {
    if (!IsAudioDeviceReady())
        return;

    for (const auto &player : players) {
        if (!player.active) {
            continue;
        }
        Character::CharacterDef character = Character::GetCharacterDef(player.characterId);
        m_bulletHitSounds[player.characterId] = LoadSound(character.bullet.bulletSoundLocation);
        m_characterBulletHitIndex[player.characterId] = 0;

        for (int i = 0; i < BULLET_HIT_POOL_SIZE; ++i) {
            m_bulletHitSoundAliases[player.characterId][i] = LoadSoundAlias(m_bulletHitSounds[player.characterId]);
        }
    }
}

void AudioSystem::UnloadMatch() { SafeUnload(m_countdownSound); }

void AudioSystem::UnloadLobby() { m_countdownSub = {}; }

void AudioSystem::UnloadGamePlay() {
    for (Sound &s : m_hitmarkerAliases) {
        SafeUnloadAlias(s);
    }
    for (Sound &s : m_hurtGruntAliases) {
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
    for (Sound &s : m_wallInputDeniedAliases) {
        SafeUnloadAlias(s);
    }

    SafeUnload(m_goBellSound);
    m_goBellLoaded = false;

    SafeUnload(m_hitmarkerSound);
    SafeUnload(m_deathSound);
    SafeUnload(m_killRewardSound);
    SafeUnload(m_hurtGruntSound);

    SafeUnload(m_wallPlacedConcreteSound);
    SafeUnload(m_wallPlacedKickDrumSound);
    SafeUnload(m_wallInputDeniedSound);

    UnloadCharacterBulletSounds();

    m_gameplaySubs.clear();
}

void AudioSystem::UnloadCharacterBulletSounds() {
    for (auto &[characterId, aliases] : m_bulletHitSoundAliases) {
        for (auto &alias : aliases) {
            SafeUnloadAlias(alias);
        }
    }
    for (auto &[characterId, sound] : m_bulletHitSounds) {
        SafeUnload(sound);
    }
}

void AudioSystem::Unload() {
    UnloadMatch();
    UnloadLobby();
    UnloadGamePlay();
}

void AudioSystem::SafeLoad(Sound &s, const char *filename) {
    if (s.stream.buffer != nullptr) {
        s = LoadSound(filename);
    }
}

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

float AudioSystem::GetMasterVolume() { return m_masterVolume; }

float AudioSystem::GetMusicVolume() { return m_musicVolume; }

float AudioSystem::GetEffectsVolume() { return m_effectsVolume; }

void AudioSystem::SetMasterVolume(float volume) { m_masterVolume = volume; }

void AudioSystem::SetMusicVolume(float volume) { m_musicVolume = volume; }

void AudioSystem::SetEffectsVolume(float volume) { m_effectsVolume = volume; }

void AudioSystem::OnCountdown(const client::GameStartingEvent &e) {
    if (e.prevCountdown == e.countdown || e.countdown > 3)
        return;

    if (e.countdown == 0) {
        if (m_goBellLoaded) {
            Play(m_goBellSound, SoundCategory::Effects);
        }
        return;
    }

    float pitch = GetCountdownPitch(e.countdown, e.max);
    SetSoundPitch(m_countdownSound, pitch);
    Play(m_countdownSound, SoundCategory::Effects);
}

void AudioSystem::OnHit(const client::HitEvent &e) {
    if (e.attackerId == e.localPlayerId) {
        PlayHitmarker();
    }
    Sound gruntSound = m_hurtGruntAliases[m_hurtGruntIndex];
    m_hurtGruntIndex = (m_hurtGruntIndex + 1) % HURT_GRUNT_POOL_SIZE;
    float pitch = 0.95f + (GetRandomValue(0, 10) / 100.0f);

    SetSoundPitch(gruntSound, pitch);
    PlaySpatialSound2D(gruntSound, e.victimPosition, m_hearingDistance, e.localPlayerPosition, SoundCategory::Effects,
                       0.2);
}

void AudioSystem::OnBulletDestroyed(const client::BulletDestroyedEvent &e) {
    Sound bulletAlias = m_bulletHitSoundAliases[e.characterId][m_characterBulletHitIndex[e.characterId]];
    m_characterBulletHitIndex[e.characterId] = (m_characterBulletHitIndex[e.characterId] + 1) % BULLET_HIT_POOL_SIZE;
    float pitch = GetPitch();

    SetSoundPitch(bulletAlias, pitch);
    PlaySpatialSound2D(bulletAlias, e.position, m_hearingDistance, e.localPlayerPosition, SoundCategory::Effects, 0.5);
}

void AudioSystem::OnPlayerDied(const client::PlayerDiedEvent &e) {
    if (e.data.killer.id == e.localPlayer.id)
        Play(m_killRewardSound, SoundCategory::Effects);

    PlaySpatialSound2D(m_deathSound, e.data.victim.position, m_hearingDistance, e.localPlayer.position,
                       SoundCategory::Effects);
}

void AudioSystem::OnWallPlaced(const client::WallPlacedEvent &event) {
    float concreteSoundVolume = 0.2f;
    Sound &concreteAlias = m_wallPlacementConcreteAliases[m_wallPlacementIndex];
    Sound &kickDrumAlias = m_wallPlacementKickDrumAliases[m_wallPlacementIndex];
    m_wallPlacementIndex = (m_wallPlacementIndex + 1) % WALL_PLACEMENT_POOL_SIZE;

    float pitch = GetPitch();

    SetSoundPitch(concreteAlias, pitch);
    SetSoundPitch(kickDrumAlias, pitch);

    Vector2 wallPlacedPosition = Map::GridToWorld(event.gridPos);
    PlaySpatialSound2D(concreteAlias, wallPlacedPosition, m_hearingDistance, event.localPlayerPosition,
                       SoundCategory::Effects, concreteSoundVolume);
    PlaySpatialSound2D(kickDrumAlias, wallPlacedPosition, m_hearingDistance, event.localPlayerPosition,
                       SoundCategory::Effects);
}

void AudioSystem::OnWallPickedUp(const client::WallPickedUpEvent &event) {
    float pickUpSoundVolume = 0.2f;
    Sound &alias = m_wallPickedUpAliases[m_wallPickedUpIndex];
    m_wallPickedUpIndex = (m_wallPickedUpIndex + 1) % WALL_PICKEDUP_POOL_SIZE;
    float pitch = GetPitch();

    SetSoundPitch(alias, pitch);

    Vector2 wallPickedUpPosition = Map::GridToWorld(event.gridPos);
    PlaySpatialSound2D(alias, wallPickedUpPosition, m_hearingDistance, event.localPlayerPosition,
                       SoundCategory::Effects, pickUpSoundVolume);
}

void AudioSystem::OnWallInputDenied(const event::WallInputDeniedEvent &e) {
    Sound &alias = m_wallInputDeniedAliases[m_wallInputDeniedIndex];
    m_wallInputDeniedIndex = (m_wallInputDeniedIndex + 1) % WALL_INPUT_DENIED_POOL_SIZE;

    Play(alias, SoundCategory::Effects, 0.5);
}

void AudioSystem::PlayHitmarker() {
    Sound &alias = m_hitmarkerAliases[m_hitmarkerIndex];
    m_hitmarkerIndex = (m_hitmarkerIndex + 1) % HITMARKER_POOL_SIZE;

    float pitch = GetPitch();

    SetSoundPitch(alias, pitch);

    Play(alias, SoundCategory::Effects);
}

float AudioSystem::GetPitch() { return 0.95f + (GetRandomValue(0, 10) / 100.0f); }

float AudioSystem::GetCountdownPitch(int value, int maxValue) {
    float t = 1.0f - (float(value - 1) / float(maxValue - 1));
    t = t * t; // ease-in
    return 0.9f + t * 0.2f;
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
