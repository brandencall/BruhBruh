#pragma once
#include "../../network/packets/gameplay_packets.hpp"
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
    int Spawn(const BulletSpawnDef &bulletDef) override;
    void Draw();
    void Draw(const state::ClientBulletState &bullet);
    void AssignId(int slot, uint32_t id);
    // void Update(float dt);
    void Update(float dt, std::array<state::PlayerState, MAX_PLAYERS> &players,
                std::unordered_map<Map::Vector2i, Map::DynamicWall, Map::GridHash> &dynamicWalls) override;
    int SpawnFromServerEvent(const network::BulletSpawnPacket &bullet);
    void ResolveLocalPredictedBullet(const network::BulletSpawnPacket &bullet, uint32_t ownerId,
                                     std::array<state::PlayerState, MAX_PLAYERS> &players,
                                     std::unordered_map<Map::Vector2i, Map::DynamicWall, Map::GridHash> &dynamicWalls);

  protected:
    void OnSpawn(state::ClientBulletState &bullet, Vector2 position, Character::CharacterId characterId) override;
    void OnBulletDestroyed(int slot, Vector2 position, Character::CharacterId characterId) override;
    void OnBulletUpdate(state::ClientBulletState &bullet, float dt) override;

  private:
    static constexpr float BULLET_INTERP_SPEED = 5.0f;
    std::unordered_map<Character::CharacterId, Texture2D> m_textures;
    std::vector<HitEffect> m_hitEffects;
    std::unordered_map<uint32_t, state::ClientBulletState> m_predictedBullets;
};
} // namespace System
