#pragma once

#include "../event_bus.hpp"
#include "../events.hpp"
#include "raylib.h"

namespace System {
class AudioSystem {

  public:
    void Init(Client::EventBus<client::HitEvent> &hitBus, Client::EventBus<client::PlayerDiedEvent> &deathBus);
    void PlaySpatialSound2D(Sound sound, Vector2 soundPos, float maxRange, Vector2 localPlayerPos);
    void Play(Sound sound);
    void Unload();

  private:
    void OnHit(const client::HitEvent &e);
    void OnPlayerDied(const client::PlayerDiedEvent &e);
    void PlayHitmarker();
    float GetSpatialVolume2D(Vector2 soundPos, float maxRange, Vector2 localPlayerPos);
    float GetSpatialPan2D(Vector2 soundPos, Vector2 localPlayerPos);

  private:
    Client::Subscription m_hitSub;
    Client::Subscription m_deathSub;
    Client::EventBus<client::HitEvent> m_hitBus;

    Sound m_hitmarkerSound;
    static constexpr int HITMARKER_POOL_SIZE = 8;
    Sound m_hitmarkerAliases[HITMARKER_POOL_SIZE];
    int m_hitmarkerIndex = 0;

    Sound m_deathSound;
    Sound m_killRewardSound;
};
} // namespace System
