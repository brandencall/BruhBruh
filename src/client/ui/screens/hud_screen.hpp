#pragma once
#include "../../../shared/state/player_state.hpp"
#include "../../event_hub.hpp"
#include "../../systems/kill_feed.hpp"
#include "../ui_manager.hpp"
#include "raylib.h"

namespace UI {

class HudScreen : public UIScreen {
  public:
    HudScreen(const state::PlayerState &localPlayer, const float &gameTime, Client::EventHub &events);

    void Update(float dt) override;
    void Render() override;
    bool BlocksGameInput() const override { return false; }
    bool IsDone() const override { return false; }

  private:
    void RenderHealthBar(const Rectangle &container, const std::string &healthText, int padding, int barHeight,
                         int barWidth, int fontSize);
    void RenderWalls(const Rectangle &container, int padding, int barHeight, int fontSize);
    void RenderGameTime(int screenW);

  private:
    const state::PlayerState &m_localPlayer;
    const float &m_gameTime;
    Client::EventHub &m_events;

    System::KillFeed m_killFeed;

    Client::Subscription m_deathSub;
};

} // namespace UI
