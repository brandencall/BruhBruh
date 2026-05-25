#pragma once
#include "../config.hpp"
#include "../shared/characters/character_types.hpp"
#include "../shared/map/dynamic_walls/wall_manager.hpp"
#include "../shared/map/grid.hpp"
#include "../shared/map/map_types.hpp"
#include "../shared/state/bullet_state.hpp"
#include "../shared/state/player_state.hpp"
#include "../shared/systems/ability_system.hpp"
#include "../shared/systems/bullet_system.hpp"
#include "event_bus.hpp"
#include "map/server_wall_manager.hpp"
#include "systems/server_bullet_system.hpp"
#include <array>
#include <stdint.h>

class GameSimulation {
  public:
    GameSimulation() = default;
    void Initialize(EventBus &eventBus);
    void Reset();
    void SetupBulletSystem();
    void HandlePlayerDied(state::PlayerState &player, uint32_t shooterId);
    void SetupWallManager();
    void Update(float tickRate);
    void RespawnPlayer(state::PlayerState &player);
    void ApplyInput(uint32_t playerId, Character::CharacterId characterId, const state::PlayerInput &input);
    const std::array<state::PlayerState, MAX_PLAYERS> &GetPlayers() const;
    const std::array<state::BulletState, MAX_BULLETS> &GetBullets();
    void CreatePlayer(uint32_t playerId, Character::CharacterId characterId, const char *name);
    void RemovePlayer(uint32_t playerId);

    float GetGameTime() const;

    System::BulletSystem<state::BulletState> &GetBulletSystem();
    Map::WallManager &GetWallManager();
    const Map::WallManager &GetWallManager() const;

  private:
    void HandleWallInput(state::PlayerState &player, const state::PlayerInput &input,
                         const Character::CharacterDef &charDef);
    bool TryPlaceWall(state::PlayerState &player, Map::Vector2i gridPos);

  private:
    float m_gameTime = MATCH_TIME;
    Map::MapData m_map;
    EventBus *m_eventBus = nullptr;
    std::array<state::PlayerState, MAX_PLAYERS> m_players;
    System::ServerBulletSystem m_bulletSystem;
    System::AbilitySystem m_abilitySystem;
    Map::ServerWallManager m_wallManager;
};
