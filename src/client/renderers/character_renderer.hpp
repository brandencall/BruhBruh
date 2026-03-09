#pragma once

#include "../../shared/characters/character_types.hpp"
#include "../../shared/state/player_state.hpp"
#include <cstdint>
#include <map>
#include <unordered_map>

class CharacterRenderer {
  public:
    void Load();
    void Unload();
    void Sync(const state::PlayerState &state, float dt);
    void Draw(const state::PlayerState &player);
    void SnapToPosition(const state::PlayerState &state);
    Vector2 GetPosition(uint32_t playerId);

  private:
    std::unordered_map<Character::CharacterId, Texture2D> m_textures;
    std::map<uint32_t, Vector2> m_positions;
    std::map<uint32_t, Vector2> m_targetPosition;
};
