#pragma once
#include "../../../shared/state/player_state.hpp"
#include "../ui_manager.hpp"
#include <array>

namespace UI {

struct GameEndData {
    int countdown;
    int playerCount;
    std::array<state::RankedPlayer, MAX_PLAYERS> rankings;
};

class GameEndScreen : public UIScreen {
  public:
    explicit GameEndScreen(const GameEndData &data);

    void Update(float dt) override {};
    void Render() override;
    bool BlocksGameInput() const override;
    bool IsDone() const override;

    void UpdateCountdown(float serverCountdown);

  private:
    void RenderBackground() const;
    void RenderTitle() const;
    void RenderRankings() const;
    void RenderCountdown() const;

    float m_countdown;
    uint8_t m_playerCount;
    std::array<state::RankedPlayer, MAX_PLAYERS> m_rankings;
};

} // namespace UI
