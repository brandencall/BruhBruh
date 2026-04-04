#pragma once
#include "../../../shared/state/player_state.hpp"
#include "../../event_hub.hpp"
#include "../../systems/kill_feed.hpp"
#include "../ui_manager.hpp"

namespace UI {

class HudScreen : public UIScreen {
  public:
    HudScreen(const state::PlayerState &localPlayer, const float &gameTime, Client::EventHub &events);
    ~HudScreen();

    void Update(float dt) override;
    void Render() override;
    bool BlocksGameInput() const override { return false; }
    bool IsDone() const override { return false; }

  private:
    const state::PlayerState &m_localPlayer;
    const float &m_gameTime;
    Client::EventHub &m_events;

    System::KillFeed m_killFeed;

    Client::SubscriptionToken m_diedToken;
};

} // namespace UI
