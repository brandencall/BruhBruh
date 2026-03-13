#pragma once
#include "../config.hpp"
#include "../shared/events.hpp"
#include "../shared/state/player_state.hpp"
#include "../shared/systems/bullet_system.hpp"
#include "characters/character_types.hpp"
#include "map/map_types.hpp"
#include "map/wall_manager.hpp"
#include "state/bullet_state.hpp"
#include <array>
#include <stdint.h>
#include <vector>

class GameSimulation {
  public:
    GameSimulation() = default;
    void Initialize();
    void Update(float tickRate);
    void RespawnPlayer(state::PlayerState &player);
    void ApplyInput(uint32_t playerId, Character::CharacterId characterId, const state::PlayerInput &input);
    const std::array<state::PlayerState, MAX_PLAYERS> &GetPlayers();
    const std::array<state::BulletState, MAX_BULLETS> &GetBullets();
    void CreatePlayer(uint32_t playerId, Character::CharacterId characterId);
    void RemovePlayer(uint32_t playerId);

    System::BulletSystem<state::BulletState> &GetBulletSystem();
    Map::WallManager &GetWallManager();

  private:
    Map::MapData m_map;
    std::array<state::PlayerState, MAX_PLAYERS> m_players;
    System::BulletSystem<state::BulletState> m_bulletSystem;
    std::vector<event::BulletSpawnEvent> m_bulletSpawnEvents;
    std::vector<event::BulletHitEvent> m_bulletHitEvents;
    Map::WallManager m_wallManager;
};
