#pragma once
#include "../ui_manager.hpp"
#include "raylib.h"

class Game;

namespace UI {

class OptionMenu : public UIScreen {
  public:
    explicit OptionMenu(Game &game);

    void Update(float dt) override;
    void Render() override;
    bool BlocksGameInput() const override { return true; }
    bool IsDone() const override { return m_done; }

  private:
    // ── Layout ───────────────────────────────────────────────────────────────
    struct Layout {
        Rectangle panel;
        Rectangle closeBtn;
    };

    void ComputeLayout();

    // ── Drawing ──────────────────────────────────────────────────────────────
    void DrawBackdrop(float ease) const;
    void DrawPanel(float ease) const;
    void DrawHeader(float ease) const;
    void DrawSlider(float ease, int index, const char *label, float value, const Rectangle &track,
                    const Rectangle &thumb, bool hovered) const;
    void DrawCloseButton(float ease) const;
    void DrawCorners(Rectangle r, Color c, float len, float thick) const;

    // ── Helpers ───────────────────────────────────────────────────────────────
    static float EaseOutQuart(float t);
    // Convert a normalised value [0,1] to a thumb x position on the track
    float ThumbX(const Rectangle &track, float value) const;
    // Convert a raw x position on the track back to a [0,1] value
    float XToValue(const Rectangle &track, float x) const;

    // ── State ─────────────────────────────────────────────────────────────────
    Game &m_game;
    bool m_done = false;
    float m_openAnim = 0.f;

    // Volume values [0,1]
    float m_masterVol = 1.f;
    float m_musicVol = 1.f;
    float m_effectsVol = 1.f;

    // Which slider is being dragged (-1 = none)
    int m_dragging = -1;

    // Cached layout — recomputed when window size changes
    Layout m_layout = {};
    int m_cachedW = 0;
    int m_cachedH = 0;

    // Scaled sizing (computed in ComputeLayout)
    int m_panelW = 0;
    int m_panelH = 0;
    int m_padX = 0;
    int m_padTop = 0;
    int m_headerH = 0;
    int m_rowH = 0; // height of one slider row
    int m_labelSz = 0;
    int m_titleSz = 0;

    // ── Style constants ───────────────────────────────────────────────────────
    static constexpr Color kAccent = {45, 80, 160, 255};
    static constexpr Color kAccentHover = {120, 160, 255, 255};
    static constexpr Color kBorder = {70, 70, 100, 255};
    static constexpr Color kTextPrimary = {255, 255, 255, 255};
    static constexpr Color kTextMuted = {160, 160, 180, 255};
    static constexpr Color kFill = {18, 18, 28, 245};
    static constexpr float kCornerLen = 10.f;
    static constexpr float kCornerThick = 2.f;
    static constexpr float kDivH = 1.f;
    static constexpr float kTrackH = 4.f;
    static constexpr float kThumbR = 8.f;
};

} // namespace UI
