#pragma once
#include "../../../shared/state/player_state.hpp"
#include "../../systems/kill_feed.hpp"
#include "../ui_manager.hpp"
#include <vector>

namespace UI {

class HudScreen : public UIScreen {
  public:
    explicit HudScreen(const state::PlayerState &localPlayer, const std::vector<System::Feed> &feed);

    void Update(float dt) override {}
    void Render() override;
    bool BlocksGameInput() const override { return false; }
    bool IsDone() const override { return false; }

  private:
    const state::PlayerState &m_localPlayer;
    const std::vector<System::Feed> &m_feed;
};

} // namespace UI
