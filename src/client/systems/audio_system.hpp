#pragma once

#include "../../shared/events.hpp"
#include "../../shared/state/player_state.hpp"
#include "../event_bus.hpp"
#include "../events.hpp"
#include "raylib.h"

namespace System {
class AudioSystem {

  public:
    AudioSystem(state::PlayerState &localPlayer);

    void Init(Client::EventBus<client::HitEvent> &hitBus, Client::EventBus<event::PlayerDiedEvent> &deathBus);
    void PlaySpatialSound2D(Sound sound, Vector2 soundPos, float maxRange);
    void Play(Sound sound);
    void Unload();

  private:
    void OnHit(const client::HitEvent &e);
    void OnPlayerDied(const event::PlayerDiedEvent &e);
    void PlayHitmarker();
    float GetSpatialVolume2D(Vector2 soundPos, float maxRange);
    float GetSpatialPan2D(Vector2 soundPos);

  private:
    state::PlayerState &m_localPlayer;

    Sound m_hitmarkerSound;
    static constexpr int HITMARKER_POOL_SIZE = 8;
    Sound m_hitmarkerAliases[HITMARKER_POOL_SIZE];
    int m_hitmarkerIndex = 0;

    Sound m_deathSound;
};
} // namespace System
