#pragma once
#include "../../../shared/state/player_state.hpp"
#include "../ui_manager.hpp"

namespace UI {

class HudScreen : public UIScreen {
  public:
    explicit HudScreen(const state::PlayerState &localPlayer);

    void Update(float dt) override {}
    void Render() override;
    bool BlocksGameInput() const override { return false; }
    bool IsDone() const override { return false; }

  private:
    const state::PlayerState &m_localPlayer;
};

} // namespace UI
