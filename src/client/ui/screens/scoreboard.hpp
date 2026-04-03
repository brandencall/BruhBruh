#pragma once
#include "../../../shared/state/player_state.hpp"
#include "../config.hpp"
#include "../ui_manager.hpp"
#include "raylib.h"
#include <array>

namespace UI {

// TODO: may want to consider break out the player score so that the scoarboard doesn't take the whole player
class Scoreboard : public UIScreen {
  public:
    explicit Scoreboard(const std::array<state::PlayerState, MAX_PLAYERS> &players);

    void Update(float dt) override {}
    void Render() override;
    bool BlocksGameInput() const override { return false; }
    bool IsDone() const override { return !IsKeyDown(KEY_TAB); }

  private:
    void SortPlayerArray(state::PlayerState *sorted);
    void DrawTextCentered(const char *text, float cx, float y, int fontSize, Color color);
    void DrawPanelBackground(float totalH, float panelX, float panelY);
    void DrawTitleBar(float panelX, float panelY);
    void DrawHeaderRow(float panelX, float panelY, float headerY, float divY);
    void DrawPlayerRows(float panelX, float divY, state::PlayerState *sorted);
    void DrawVerticalColDividers(float panelX, float panelY, float totalH);

  private:
    const std::array<state::PlayerState, MAX_PLAYERS> &m_players;
};

} // namespace UI
