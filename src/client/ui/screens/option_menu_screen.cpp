#include "option_menu_screen.hpp"
#include "../../systems/audio_system.hpp"
#include "../game.hpp"
#include "raylib.h"
#include <algorithm>
#include <cmath>
#include <string>

namespace UI {

// ─────────────────────────────────────────────────────────────────────────────
// Construction
// ─────────────────────────────────────────────────────────────────────────────

OptionMenu::OptionMenu(Game &game) : m_game(game) {
    // Initialise sliders from current engine state
    // (AudioSystem returns the live value so the slider matches whatever is
    //  already playing when the menu opens.)
    auto *audio = m_game.GetAudioSystem();
    if (audio) {
        // Replace these with getters once AudioSystem exposes them.
        m_masterVol = audio->GetMasterVolume();
        m_musicVol = audio->GetMusicVolume();
        m_effectsVol = audio->GetEffectsVolume();
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Helpers
// ─────────────────────────────────────────────────────────────────────────────

float OptionMenu::EaseOutQuart(float t) {
    t = 1.f - t;
    return 1.f - t * t * t * t;
}

float OptionMenu::ThumbX(const Rectangle &track, float value) const { return track.x + value * track.width; }

float OptionMenu::XToValue(const Rectangle &track, float x) const {
    float v = (x - track.x) / track.width;
    return std::clamp(v, 0.f, 1.f);
}

// ─────────────────────────────────────────────────────────────────────────────
// Layout
// ─────────────────────────────────────────────────────────────────────────────

void OptionMenu::ComputeLayout() {
    int sw = GetScreenWidth();
    int sh = GetScreenHeight();
    if (sw == m_cachedW && sh == m_cachedH)
        return;
    m_cachedW = sw;
    m_cachedH = sh;

    // Sizing — mirrors pause menu proportions
    m_panelW = std::max(400, std::min(680, static_cast<int>(sw * 0.38f)));
    m_padX = std::max(28, static_cast<int>(m_panelW * 0.09f));
    m_padTop = std::max(24, static_cast<int>(sh * 0.034f));
    m_headerH = std::max(56, static_cast<int>(sh * 0.082f));
    m_rowH = std::max(72, static_cast<int>(sh * 0.110f)); // room for label + track + value
    m_labelSz = std::max(16, static_cast<int>(sh * 0.026f));
    m_titleSz = std::max(26, static_cast<int>(sh * 0.050f));

    // Number of slider rows: Master, Music, Effects
    constexpr int kSliderCount = 3;

    // Close button at the bottom
    int closeBtnH = std::max(44, static_cast<int>(sh * 0.072f));
    int closeBtnW = std::max(120, static_cast<int>(m_panelW * 0.38f));

    m_panelH = m_padTop + m_headerH + static_cast<int>(kDivH) + 12 + kSliderCount * m_rowH +
               16                // gap before close button
               + closeBtnH + 20; // bottom pad

    int px = sw / 2 - m_panelW / 2;
    int py = sh / 2 - m_panelH / 2;

    m_layout.panel = {static_cast<float>(px), static_cast<float>(py), static_cast<float>(m_panelW),
                      static_cast<float>(m_panelH)};

    m_layout.closeBtn = {static_cast<float>(px + (float)m_panelW / 2 - (float)closeBtnW / 2),
                         static_cast<float>(py + m_panelH - closeBtnH - 16), static_cast<float>(closeBtnW),
                         static_cast<float>(closeBtnH)};
}

// ─────────────────────────────────────────────────────────────────────────────
// Update
// ─────────────────────────────────────────────────────────────────────────────

void OptionMenu::Update(float /*dt*/) {
    auto *audio = m_game.GetAudioSystem();
    m_openAnim = std::min(1.f, m_openAnim + GetFrameTime() * 7.f);

    ComputeLayout();

    // ── Build slider rects for hit-testing ───────────────────────────────────
    struct SliderRow {
        float *value;
        Rectangle track;
        Rectangle thumb;
    };

    auto makeTrack = [&](int rowIndex) -> Rectangle {
        float y = m_layout.panel.y + m_padTop + m_headerH + kDivH + 12 + rowIndex * m_rowH + m_rowH * 0.58f;
        return {m_layout.panel.x + m_padX, y - kTrackH * 0.5f, m_layout.panel.width - m_padX * 2.f, kTrackH};
    };
    auto makeThumb = [&](const Rectangle &track, float value) -> Rectangle {
        return {ThumbX(track, value) - kThumbR, track.y + track.height * 0.5f - kThumbR, kThumbR * 2.f, kThumbR * 2.f};
    };

    Rectangle masterTrack = makeTrack(0);
    Rectangle musicTrack = makeTrack(1);
    Rectangle effectsTrack = makeTrack(2);

    SliderRow rows[] = {
        {&m_masterVol, masterTrack, makeThumb(masterTrack, m_masterVol)},
        {&m_musicVol, musicTrack, makeThumb(musicTrack, m_musicVol)},
        {&m_effectsVol, effectsTrack, makeThumb(effectsTrack, m_effectsVol)},
    };
    constexpr int kRowCount = static_cast<int>(sizeof(rows) / sizeof(rows[0]));

    Vector2 mouse = GetMousePosition();

    // ── Drag logic ───────────────────────────────────────────────────────────
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        for (int i = 0; i < kRowCount; i++) {
            // Widen the thumb hit area slightly for easier grabbing
            Rectangle bigThumb = rows[i].thumb;
            bigThumb.x -= 6.f;
            bigThumb.y -= 6.f;
            bigThumb.width += 12.f;
            bigThumb.height += 12.f;

            // Also allow clicking anywhere on the track to jump
            if (CheckCollisionPointRec(mouse, bigThumb) || CheckCollisionPointRec(mouse, rows[i].track)) {
                m_dragging = i;
            }
        }
    }

    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
        m_dragging = -1;

    if (m_dragging >= 0) {
        float newVal = XToValue(rows[m_dragging].track, mouse.x);
        *rows[m_dragging].value = newVal;

        if (audio) {
            if (m_dragging == 0)
                audio->SetMasterVolume(m_masterVol);
            if (m_dragging == 1)
                audio->SetMusicVolume(m_musicVol);
            if (m_dragging == 2)
                audio->SetEffectsVolume(m_effectsVol);
        }
    }

    if (IsKeyPressed(KEY_ESCAPE)) {
        m_done = true;
        if (audio) {
            audio->Save();
        }
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(mouse, m_layout.closeBtn)) {
        m_done = true;
        if (audio) {
            audio->Save();
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Render
// ─────────────────────────────────────────────────────────────────────────────

void OptionMenu::Render() {
    ComputeLayout();
    float ease = EaseOutQuart(m_openAnim);

    DrawBackdrop(ease);
    DrawPanel(ease);
}

void OptionMenu::DrawBackdrop(float ease) const {
    int sw = GetScreenWidth();
    int sh = GetScreenHeight();

    // Dark overlay — same colour as main menu background
    DrawRectangle(0, 0, sw, sh, ColorAlpha({10, 10, 16, 255}, 0.72f * ease));

    // Vignette rings — mirrors MainMenuScene::RenderBackground()
    for (int r = sh; r > 0; r -= sh / 8) {
        unsigned char alpha = static_cast<unsigned char>(60.f * (1.f - (float)r / sh) * ease);
        DrawCircle(sw / 2, sh / 2, static_cast<float>(r), {0, 0, 0, alpha});
    }
}

void OptionMenu::DrawPanel(float ease) const {
    // Slide in from top
    float offsetY = (1.f - ease) * -30.f;
    Rectangle r = m_layout.panel;
    r.y += offsetY;

    // Panel background
    DrawRectangleRec(r, kFill);

    // Top accent bar
    DrawRectangle(static_cast<int>(r.x), static_cast<int>(r.y), static_cast<int>(r.width), 2, kAccent);

    // Border
    DrawRectangleLinesEx(r, 1.5f, kBorder);

    // Corner brackets
    DrawCorners(r, kAccent, kCornerLen, kCornerThick);

    // Scanline overlay
    for (int py = static_cast<int>(r.y); py < static_cast<int>(r.y + r.height); py += 4)
        DrawRectangle(static_cast<int>(r.x), py, static_cast<int>(r.width), 1, {0, 0, 0, 18});

    DrawHeader(ease);

    // ── Sliders ───────────────────────────────────────────────────────────────
    struct SliderDef {
        const char *label;
        float value;
        int index;
    };
    SliderDef defs[] = {
        {"Master Volume", m_masterVol, 0},
        {"Music Volume", m_musicVol, 1},
        {"Effects Volume", m_effectsVol, 2},
    };

    Vector2 mouse = GetMousePosition();

    for (auto &def : defs) {
        float trackCY = r.y + m_padTop + m_headerH + kDivH + 12 + def.index * m_rowH + m_rowH * 0.58f;
        Rectangle track = {r.x + m_padX, trackCY - kTrackH * 0.5f, r.width - m_padX * 2.f, kTrackH};
        Rectangle thumb = {ThumbX(track, def.value) - kThumbR, track.y + track.height * 0.5f - kThumbR, kThumbR * 2.f,
                           kThumbR * 2.f};
        Rectangle bigThumb = {thumb.x - 6.f, thumb.y - 6.f, thumb.width + 12.f, thumb.height + 12.f};
        bool hovered = CheckCollisionPointRec(mouse, bigThumb) || m_dragging == def.index;
        DrawSlider(ease, def.index, def.label, def.value, track, thumb, hovered);
    }

    DrawCloseButton(ease);
}

void OptionMenu::DrawHeader(float ease) const {
    float offsetY = (1.f - ease) * -30.f;
    float y = m_layout.panel.y + offsetY + m_padTop;

    // Title — centred
    const char *title = "OPTIONS";
    int titleW = MeasureText(title, m_titleSz);
    DrawText(title, static_cast<int>(m_layout.panel.x + m_layout.panel.width * 0.5f - titleW * 0.5f),
             static_cast<int>(y + 2.f), m_titleSz, kTextPrimary);

    // Divider — same style as pause menu
    float divY = y + m_headerH;
    float lineW = m_layout.panel.width * 0.60f;
    float lineX = m_layout.panel.x + (m_layout.panel.width - lineW) * 0.5f;
    DrawRectangle(static_cast<int>(lineX), static_cast<int>(divY), static_cast<int>(lineW), 1, {70, 70, 90, 255});
}

void OptionMenu::DrawSlider(float /*ease*/, int /*index*/, const char *label, float value, const Rectangle &track,
                            const Rectangle &thumb, bool hovered) const {
    // Row label — left aligned, above the track
    float labelY = track.y - m_labelSz - 10.f;
    DrawText(label, static_cast<int>(track.x), static_cast<int>(labelY), m_labelSz, kTextPrimary);

    // Percentage value — right aligned on the same line as the label
    std::string pct = std::to_string(static_cast<int>(std::round(value * 100.f))) + "%";
    int pctW = MeasureText(pct.c_str(), m_labelSz);
    DrawText(pct.c_str(), static_cast<int>(track.x + track.width - pctW), static_cast<int>(labelY), m_labelSz,
             kTextMuted);

    // Track background (full width, dark)
    DrawRectangleRec(track, {30, 30, 50, 255});
    DrawRectangleLinesEx(track, 1.f, kBorder);

    // Filled portion (left of thumb) — blue, matching button primary colour
    Rectangle filled = track;
    filled.width = ThumbX(track, value) - track.x;
    if (filled.width > 0.f)
        DrawRectangleRec(filled, kAccent);

    // Thumb circle
    Color thumbFill = hovered ? Color{70, 120, 220, 255} : Color{45, 80, 160, 255};
    Color thumbBorder = hovered ? kAccentHover : kBorder;
    float cx = thumb.x + thumb.width * 0.5f;
    float cy = thumb.y + thumb.height * 0.5f;
    DrawCircle(static_cast<int>(cx), static_cast<int>(cy), kThumbR, thumbFill);
    DrawCircleLines(static_cast<int>(cx), static_cast<int>(cy), kThumbR, thumbBorder);
}

void OptionMenu::DrawCloseButton(float ease) const {
    float offsetY = (1.f - ease) * -30.f;
    Rectangle btn = m_layout.closeBtn;
    btn.y += offsetY;

    Vector2 mouse = GetMousePosition();
    bool hovered = CheckCollisionPointRec(mouse, btn);

    Color fill = hovered ? Color{70, 120, 220, 255} : Color{45, 80, 160, 255};
    Color border = hovered ? kAccentHover : kBorder;

    DrawRectangleRec(btn, fill);
    DrawRectangleLinesEx(btn, 1.5f, border);

    const char *label = "CLOSE";
    int lw = MeasureText(label, m_labelSz);
    DrawText(label, static_cast<int>(btn.x + btn.width * 0.5f - lw * 0.5f),
             static_cast<int>(btn.y + btn.height * 0.5f - m_labelSz * 0.5f), m_labelSz, WHITE);
}

// ─────────────────────────────────────────────────────────────────────────────
// Corner brackets
// ─────────────────────────────────────────────────────────────────────────────

void OptionMenu::DrawCorners(Rectangle r, Color c, float len, float thick) const {
    int t = static_cast<int>(thick);
    int L = static_cast<int>(len);
    int x0 = static_cast<int>(r.x) - 1;
    int y0 = static_cast<int>(r.y) - 1;
    int x1 = static_cast<int>(r.x + r.width);
    int y1 = static_cast<int>(r.y + r.height);

    DrawRectangle(x0, y0, L, t, c);
    DrawRectangle(x0, y0, t, L, c);
    DrawRectangle(x1 - L, y0, L, t, c);
    DrawRectangle(x1 - t, y0, t, L, c);
    DrawRectangle(x0, y1 - t, L, t, c);
    DrawRectangle(x0, y1 - L, t, L, c);
    DrawRectangle(x1 - L, y1 - t, L, t, c);
    DrawRectangle(x1 - t, y1 - L, t, L, c);
}

} // namespace UI
