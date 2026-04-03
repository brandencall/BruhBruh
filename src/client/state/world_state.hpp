#pragma once

#include "../../config.hpp"
#include "../shared/state/player_state.hpp"
#include "map/map_types.hpp"
#include "map/tiles/tilemap_loader.hpp"
#include <array>
#include <cstdint>

// This might just need to be a struct instead of a class
class ClientWorldState {
  public:
    uint32_t m_currentPlayerId = -1;
    float m_lobbyTime = -1;
    std::array<state::PlayerState, MAX_PLAYERS> m_players;
    Map::MapData m_map;
    Map::TileMap m_tileMap;
};
