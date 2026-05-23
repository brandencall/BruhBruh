#pragma once
#include "../event_bus.hpp"
#include "../events.hpp"
#include "raylib.h"

namespace System {

class CharacterCamera {

  public:
    void Init(Client::EventBus<client::HitEvent> &hitBus, Client::EventBus<client::PlayerDiedEvent> &deathBus,
              Client::EventBus<client::WallPlacedEvent> &wallBus);
    void Unload();
    void Update(float dt);
    void RenderDamageShader(RenderTexture2D texture);
    void SetPosition(Vector2 position);
    const Camera2D *GetCamera() const;

  private:
    void OnHit(const client::HitEvent &e);
    void OnPlayerDied(const client::PlayerDiedEvent &e);
    void OnWallPlaced(const client::WallPlacedEvent &e);
    void AddShake(float amount);

  private:
    Client::Subscription m_hitSub;
    Client::Subscription m_deathSub;
    Client::Subscription m_wallSub;

    Camera2D m_camera;
    float m_shake = 0.0f;

    float m_maxShakeOffset = 12.0f;
    float m_shakeDecay = 2.0f;

    Vector2 m_baseOffset;

    Shader m_damageShader;
    int m_damageLoc;
    float m_damageEffect;
};

} // namespace System
