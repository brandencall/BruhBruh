#pragma once
#include "../config.hpp"
#include "../shared/state/player_state.hpp"
#include "../shared/systems/bullet_system.hpp"
#include "characters/character_types.hpp"
#include "event_bus.hpp"
#include "map/map_types.hpp"
#include "map/server_wall_manager.hpp"
#include "map/wall_manager.hpp"
#include "state/bullet_state.hpp"
#include "systems/server_bullet_system.hpp"
#include <array>
#include <stdint.h>

class GameSimulation {
  public:
    GameSimulation() = default;
    void Initialize(EventBus &eventBus);
    void SetupBulletSystem();
    void SetupWallManager();
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
    EventBus *m_eventBus = nullptr;
    std::array<state::PlayerState, MAX_PLAYERS> m_players;
    System::ServerBulletSystem m_bulletSystem;
    Map::ServerWallManager m_wallManager;
};
