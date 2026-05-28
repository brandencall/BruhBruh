#pragma once
#include "../../../shared/state/player_state.hpp"
#include "../../event_hub.hpp"
#include "../../renderers/ability_renderer.hpp"
#include "../../systems/kill_feed.hpp"
#include "../ui_manager.hpp"
#include "raylib.h"

namespace UI {

class HudScreen : public UIScreen {
  public:
    HudScreen(const state::PlayerState &localPlayer, const float &gameTime, Client::EventHub &events,
              const Render::AbilityRenderer abilityRenderer);

    void Update(float dt) override;
    void Render() override;
    bool BlocksGameInput() const override { return false; }
    bool IsDone() const override { return false; }

  private:
    void RenderPickupPanel(int screenW, int screenH, int padding);
    void RenderHealthBar(const Rectangle &container, const std::string &healthText, int padding, int barHeight,
                         int barWidth, int fontSize);
    void RenderWalls(const Rectangle &container, int padding, int barHeight, int fontSize);
    void RenderWallCooldown(int x, int y, int padding, int fontSize, int barHeight);
    void RenderGameTime(int screenW);

  private:
    const state::PlayerState &m_localPlayer;
    const float &m_gameTime;
    Client::EventHub &m_events;
    Render::AbilityRenderer m_abilityRenderer;

    System::KillFeed m_killFeed;

    Client::Subscription m_deathSub;
};

} // namespace UI
