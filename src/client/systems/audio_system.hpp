#pragma once

#include "../event_bus.hpp"
#include "../events.hpp"
#include "raylib.h"
#include <string>

namespace System {

enum class SoundCategory { Music, Effects };

class AudioSystem {
  public:
    void Load();
    void Save();
    std::string ConfigPath();

    void InitGamePlay(Client::EventBus<client::HitEvent> &hitBus, Client::EventBus<client::PlayerDiedEvent> &deathBus,
                      Client::EventBus<client::WallPlacedEvent> &wallPlacedBus,
                      Client::EventBus<client::WallPickedUpEvent> &wallPickedUpBus);

    void PlaySpatialSound2D(Sound sound, Vector2 soundPos, float maxRange, Vector2 localPlayerPos,
                            SoundCategory category, float soundVolume = 1.0f);
    void Play(Sound sound, SoundCategory category, float soundVolume = 1.0f);
    void UnloadGamePlay();
    void Unload();

    float GetMasterVolume();
    float GetMusicVolume();
    float GetEffectsVolume();

    void SetMasterVolume(float volume);
    void SetMusicVolume(float volume);
    void SetEffectsVolume(float volume);

  private:
    void SafeUnload(Sound &s);
    void SafeUnloadAlias(Sound &s);

    void OnHit(const client::HitEvent &e);
    void OnPlayerDied(const client::PlayerDiedEvent &e);
    void OnWallPlaced(const client::WallPlacedEvent &e);
    void OnWallPickedUp(const client::WallPickedUpEvent &e);
    void PlayHitmarker();
    float GetSpatialVolume2D(Vector2 soundPos, float maxRange, Vector2 localPlayerPos);
    float GetSpatialPan2D(Vector2 soundPos, Vector2 localPlayerPos);

  private:
    std::vector<Client::Subscription> m_gameplaySubs;

    float m_masterVolume = 1.0f;
    float m_musicVolume = 1.0f;
    float m_effectsVolume = 1.0f;

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
