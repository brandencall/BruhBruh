#pragma once

#include "../../shared/systems/bullet_system.hpp"
#include "../state/client_bullet_state.hpp"
#include <cstdint>
#include <vector>

namespace System {

struct HitEffect {
    Vector2 position;
    float timer;
    float maxTimer;
};

class ClientBulletSystem : public BulletSystem<state::ClientBulletState> {

  public:
    void Load();
    void Unload();
    void Draw(const std::array<state::ClientBulletState, MAX_BULLETS> &bullets);
    void Draw(const state::ClientBulletState &bullet);
    void AssignId(int slot, uint32_t id);
    void Update(float dt);
    int SpawnFromServerEvent(uint32_t serverId, uint32_t ownerId, Vector2 position, Vector2 velocity,
                             const Character::CharacterDef &character);

  protected:
    void OnSpawn(state::ClientBulletState &bullet, Vector2 position, Character::CharacterId characterId) override;
    void OnBulletDestroyed(int slot, Vector2 position) override;

  private:
    static constexpr float BULLET_INTERP_SPEED = 5.0f;
    std::unordered_map<Character::CharacterId, Texture2D> m_textures;
    std::vector<HitEffect> m_hitEffects;
};
} // namespace System
