#pragma once
#include "../event_hub.hpp"
#include "../events.hpp"
#include "raylib.h"

namespace System {

class CharacterCamera {

  public:
    void Init(Client::EventHub &events);
    void Unload();
    void Update(float dt);
    void RenderDamageShader(RenderTexture2D texture);
    void SetPosition(Vector2 position);
    const Camera2D *GetCamera() const;

  private:
    void OnHit(const client::HitEvent &e);
    void OnPlayerDied(const client::PlayerDiedEvent &e);
    void OnWallPlaced(const client::WallPlacedEvent &e);
    void OnWallInputDenied(const event::WallInputDeniedEvent &e);
    void AddShake(float amount);

  private:
    std::vector<Client::Subscription> m_subscriptions;

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
