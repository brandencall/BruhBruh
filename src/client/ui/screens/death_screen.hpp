#pragma once
#include "../../../shared/state/player_state.hpp"
#include "../ui_manager.hpp"

namespace UI {

class DeathScreen : public UIScreen {
  public:
    explicit DeathScreen(const state::PlayerState &localPlayer);

    void Update(float dt) override {}
    void Render() override;
    bool BlocksGameInput() const override { return true; }
    bool IsDone() const override { return m_localPlayer.respawnTimer <= 0.0f; }

  private:
    const state::PlayerState &m_localPlayer; // ref to whatever lives in WorldState
};

} // namespace UI
