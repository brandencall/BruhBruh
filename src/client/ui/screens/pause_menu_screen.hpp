#pragma once
#include "../../game.hpp"
#include "../ui_manager.hpp"
#include "raylib.h"
#include <functional>
#include <string>
#include <vector>

namespace UI {

// ---------------------------------------------------------------------------
// MenuContext — drives which buttons are visible and what "Leave" means
// ---------------------------------------------------------------------------
enum class MenuContext {
    InGame,  // Resume | Options | Leave (→ Start)   | Quit to Desktop
    Lobby,   // Options | Leave Lobby (→ Start)       | Quit to Desktop
    MainMenu // Options | Quit to Desktop  (single combined quit button)
};

// ---------------------------------------------------------------------------
// PauseMenuButton — a single button entry with label + callback
// ---------------------------------------------------------------------------
struct PauseMenuButton {
    std::string label;
    std::function<void()> callback;
    bool isDanger = false; // red tint (quit/destructive)
};

struct PauseMenuConfig {
    MenuContext context = MenuContext::InGame;
    std::function<void()> onLeave;
    std::function<void()> onQuitDesktop;
};

class PauseMenu : public UIScreen {
  public:
    explicit PauseMenu(Game &game, UIManager &ui, const PauseMenuConfig &cfg);

    bool BlocksGameInput() const override;
    bool IsDone() const override;

    // Call every frame while the menu is open
    void Update(float _) override;
    void Render() override;

  private:
    void OnResume();
    void OnOptions();
    void OnLeave();
    void OnQuitDesktop();

    // Layout
    void BuildButtons();
    void LayoutRects();

    // Helpers
    void DrawPanel() const;
    void DrawHeader() const;
    void DrawButtons() const;
    void DrawCorners(Rectangle r, Color c, float len, float thick) const;
    std::string ContextTag() const;
    std::string TitleText() const;
    std::string LeaveHint() const;

  private:
    Game &m_game;
    UI::UIManager &m_ui;
    PauseMenuConfig m_cfg;

    std::vector<PauseMenuButton> m_buttons;
    bool m_done = false;
    int m_selectedIndex = 0; // keyboard / gamepad nav

    // Cached rects (rebuilt on LayoutRects)
    Rectangle m_panelRect{};
    int m_panelWidth = 320;
    int m_panelHeight = 0; // computed from button count

    // Animation
    float m_openAnim = 0.f;           // 0→1 on open, eases in
    mutable float m_flashTimer = 0.f; // brief flash on button press
    mutable int m_flashIndex = -1;

    // Style constants
    static constexpr float kPadX = 28.f;
    static constexpr float kPadTop = 24.f;
    static constexpr float kBtnH = 46.f;
    static constexpr float kBtnGap = 8.f;
    static constexpr float kHeaderH = 52.f;
    static constexpr float kDivH = 1.f;
    static constexpr float kSepH = 14.f; // gap before danger btn
    static constexpr float kFooterH = 38.f;
    static constexpr float kCornerLen = 10.f;
    static constexpr float kCornerThick = 2.f;
    static constexpr Color kAccent = {224, 90, 30, 255};
    static constexpr Color kDanger = {210, 50, 50, 255};
    static constexpr Color kBg = {13, 17, 23, 230};
    static constexpr Color kBgPanel = {18, 23, 32, 245};
    static constexpr Color kBorder = {255, 255, 255, 20};
    static constexpr Color kTextPrimary = {230, 230, 230, 255};
    static constexpr Color kTextMuted = {255, 255, 255, 60};
};

} // namespace UI
