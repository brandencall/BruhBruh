#pragma once

#include "../../shared/systems/bullet_system.hpp"
#include "../state/client_bullet_state.hpp"
#include <cstdint>

namespace System {

class ClientBulletSystem : public BulletSystem<state::ClientBulletState> {

  public:
    void Load();
    void Unload();
    void Draw(const state::ClientBulletState &bullet);
    void AssignId(int slot, uint32_t id);
    void Update(float dt);

  protected:
    void OnSpawn(state::ClientBulletState &bullet, Vector2 position, Character::CharacterId characterId) override;

  private:
    static constexpr float BULLET_INTERP_SPEED = 5.0f;
    std::unordered_map<Character::CharacterId, Texture2D> m_textures;
};
} // namespace System
