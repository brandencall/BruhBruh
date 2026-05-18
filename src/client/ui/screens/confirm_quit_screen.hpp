// confirm_quit_screen.hpp
#pragma once
#include "../ui_manager.hpp"
#include "raylib.h"
#include <functional>

namespace UI {

class ConfirmQuitScreen : public UIScreen {
  public:
    explicit ConfirmQuitScreen(std::function<void()> onConfirm) : m_onConfirm(std::move(onConfirm)) {}

    bool BlocksGameInput() const override;
    bool IsDone() const override;

    void Update(float dt) override;
    void Render() override;

  private:
    void ComputeLayout();

  private:
    std::function<void()> m_onConfirm;
    bool m_done = false;
    Rectangle m_panel = {};
    Rectangle m_yesBtn = {};
    Rectangle m_noBtn = {};
};

} // namespace UI
