#pragma once

#include "../event_bus.hpp"
#include "../event_hub.hpp"
#include "../events.hpp"
#include "raylib.h"
#include <array>
#include <string>

namespace System {

enum class SoundCategory { Music, Effects };

class AudioSystem {
  public:
    void Load();
    void Save();
    std::string ConfigPath();

    void LoadMatchSounds();
    void InitLobby(Client::EventBus<client::GameStartingEvent> &startingBus);
    void InitGamePlay(Client::EventHub &events);
    void InitCharacterBulletSounds(std::array<state::PlayerState, MAX_PLAYERS> &players);

    void UnloadMatch();
    void UnloadLobby();
    void UnloadGamePlay();
    void UnloadCharacterBulletSounds();
    void Unload();

    void PlaySpatialSound2D(Sound sound, Vector2 soundPos, float maxRange, Vector2 localPlayerPos,
                            SoundCategory category, float soundVolume = 1.0f);
    void Play(Sound sound, SoundCategory category, float soundVolume = 1.0f);

    float GetMasterVolume();
    float GetMusicVolume();
    float GetEffectsVolume();

    void SetMasterVolume(float volume);
    void SetMusicVolume(float volume);
    void SetEffectsVolume(float volume);

  private:
    void SafeLoad(Sound &s, const char *filename);
    void SafeUnload(Sound &s);
    void SafeUnloadAlias(Sound &s);

    void OnCountdown(const client::GameStartingEvent &e);
    void OnHit(const client::HitEvent &e);
    void OnBulletDestroyed(const client::BulletDestroyedEvent &e);
    void OnPlayerDied(const client::PlayerDiedEvent &e);
    void OnWallPlaced(const client::WallPlacedEvent &e);
    void OnWallPickedUp(const client::WallPickedUpEvent &e);

    void PlayHitmarker();

    float GetCountdownPitch(int value, int maxValue);
    float GetSpatialVolume2D(Vector2 soundPos, float maxRange, Vector2 localPlayerPos);
    float GetSpatialPan2D(Vector2 soundPos, Vector2 localPlayerPos);

  private:
    Client::Subscription m_countdownSub;
    std::vector<Client::Subscription> m_gameplaySubs;

    float m_masterVolume = 1.0f;
    float m_musicVolume = 1.0f;
    float m_effectsVolume = 1.0f;

    Sound m_countdownSound;
    Sound m_goBellSound;
    bool m_goBellLoaded = false;

    // Character Specific bullet hit sounds
    // std::unordered_map<Character::CharacterId, Texture2D> m_textures;
    static constexpr int BULLET_HIT_POOL_SIZE = 8;
    std::unordered_map<Character::CharacterId, Sound> m_bulletHitSounds;
    std::unordered_map<Character::CharacterId, std::array<Sound, BULLET_HIT_POOL_SIZE>> m_bulletHitSoundAliases;
    std::unordered_map<Character::CharacterId, int> m_characterBulletHitIndex;

    Sound m_hitmarkerSound;
    static constexpr int HITMARKER_POOL_SIZE = 8;
    Sound m_hitmarkerAliases[HITMARKER_POOL_SIZE];
    int m_hitmarkerIndex = 0;

    Sound m_deathSound;
    Sound m_killRewardSound;

    Sound m_wallPlacedConcreteSound;
    Sound m_wallPlacedKickDrumSound;
    static constexpr int WALL_PLACEMENT_POOL_SIZE = 4;
    Sound m_wallPlacementConcreteAliases[WALL_PLACEMENT_POOL_SIZE];
    Sound m_wallPlacementKickDrumAliases[WALL_PLACEMENT_POOL_SIZE];
    int m_wallPlacementIndex = 0;

    Sound m_wallPickedUpSound;
    static constexpr int WALL_PICKEDUP_POOL_SIZE = 4;
    Sound m_wallPickedUpAliases[WALL_PICKEDUP_POOL_SIZE];
    int m_wallPickedUpIndex = 0;
};
} // namespace System
