#pragma once

#include "../../shared/characters/character_types.hpp"
#include "../../shared/state/player_state.hpp"
#include <cstdint>
#include <unordered_map>

namespace Render {

class CharacterRenderer {
  public:
    void Load();
    void Unload();
    void Sync(const state::PlayerState &state, float dt);
    void Draw(const std::array<state::PlayerState, MAX_PLAYERS> &players);
    void SnapToPosition(const state::PlayerState &state);
    Vector2 GetPosition(uint32_t playerId);

  private:
    void Draw(const state::PlayerState &player);
    void DrawHealthBar(const state::PlayerState &player, Vector2 position, int frameWidth, int frameHeight);
    void DrawPlayerAura(const state::PlayerState &player);
    void DebugHitBox(const state::PlayerState &player);

  private:
    // Every Character has this many frames and directions (Could move this to be configured per character)
    static constexpr int DIR_COUNT = 4;
    static constexpr int FRAME_COUNT = 4;
    static constexpr int ROW_COUNT = 9;

    static constexpr int IDLE_ROW_OFFSET = 0;
    static constexpr int RUN_ROW_OFFSET = 4;
    static constexpr int DEAD_ROW = 8;

    std::unordered_map<Character::CharacterId, Texture2D> m_textures;
    std::unordered_map<uint32_t, Vector2> m_positions;
    std::unordered_map<uint32_t, Vector2> m_targetPosition;
    std::unordered_map<uint32_t, float> m_blinkTimers;
};

} // namespace Render
