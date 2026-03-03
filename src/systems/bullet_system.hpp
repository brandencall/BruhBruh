#pragma once
#include "../config.hpp"
#include "../entities/state/bullet_state.hpp"
#include "bullet_entity.hpp"
#include "entity.hpp"
#include <array>
#include <memory>
#include <raylib.h>
#include <vector>

namespace System {
class DamageSystem;

class BulletSystem {
  public:
    std::unique_ptr<BulletEntity> CreateBullet(Entity &entity, const Camera2D &camera);
    void Update(float dt, std::vector<std::unique_ptr<Entity>> &entities);
    void Draw(std::vector<std::unique_ptr<Entity>> &entities);

    virtual int Spawn(uint32_t ownerId, Vector2 position, Vector2 direction, float speed = 400.0f,
                      float lifetime = 3.0f);
    virtual void Update(float dt);
    virtual void Deactivate(int slot);

    std::array<state::BulletState, MAX_BULLETS> &GetBullets() { return m_bullets; }
    const state::BulletState *Get(int slot) const;

  protected:
    static uint32_t MakeId(int slot, uint16_t generation) {
        return (static_cast<uint32_t>(generation) << 16) | static_cast<uint32_t>(slot);
    }

  private:
    std::array<state::BulletState, MAX_BULLETS> m_bullets{};
    std::array<uint16_t, MAX_BULLETS> m_generations{}; // for stale ID detection
};
} // namespace System
