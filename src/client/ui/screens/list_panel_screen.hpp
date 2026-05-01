#pragma once
#include "../ui_manager.hpp"
#include "../utils/text_utils.hpp"
#include "raylib.h"
#include <algorithm>
#include <steam/steam_api.h>
#include <string>
#include <vector>

namespace UI {

inline Color PersonaStateColor(EPersonaState state) {
    switch (state) {
    case k_EPersonaStateOnline:
        return {100, 220, 120, 255};
    case k_EPersonaStateBusy:
        return {220, 100, 100, 255};
    case k_EPersonaStateAway:
    case k_EPersonaStateSnooze:
        return {200, 180, 60, 255};
    default:
        return {150, 150, 150, 255};
    }
}

inline const char *PersonaStateLabel(EPersonaState state) {
    switch (state) {
    case k_EPersonaStateOnline:
        return "Online";
    case k_EPersonaStateBusy:
        return "Busy";
    case k_EPersonaStateAway:
        return "Away";
    case k_EPersonaStateSnooze:
        return "Snooze";
    default:
        return "Online";
    }
}

// Renders the left portion of a row: status dot, name, status label.
// Returns the X coordinate immediately after the status label block so
// the subclass can place its action button relative to it.
inline void DrawRowIdentity(int x, int y, int rowH, int padding, const std::string &name, EPersonaState state) {
    int dotR = 6;
    int dotX = x + padding + dotR;
    int dotY = y + rowH / 2;
    DrawCircle(dotX, dotY, dotR, PersonaStateColor(state));

    int nameX = dotX + dotR + padding;
    int nameY = y + rowH / 2 - 10;
    DrawText(name.c_str(), nameX, nameY, 18, WHITE);
    DrawText(PersonaStateLabel(state), nameX, nameY + 20, 13, PersonaStateColor(state));
}

// ─────────────────────────────────────────────────────────────────────────────
// SteamListPanel<RowT>
//
// CRTP base that owns all panel chrome: background, header, scrollbar, scissor,
// ESC / scroll-wheel handling.  Subclasses supply:
//   - const char*  PanelTitle()    — header text
//   - void         PopulateRows()  — fill m_rows
//   - void         RenderRow(const RowT&, int x, int y, int rowW, int rowH,
//                            bool rowHovered, bool btnHovered) const
//   - void         OnRowAction(RowT&)   — called on action-button click
//   - const char*  ActionLabel(const RowT&) const  — button text
//   - bool         ActionDone(const RowT&) const   — true → show "done" state
// ─────────────────────────────────────────────────────────────────────────────

template <typename Derived, typename RowT> class SteamListPanel : public UIScreen {
  public:
    bool BlocksGameInput() const override { return true; }
    bool IsDone() const override { return m_done; }

    void Update(float dt) override {
        if (IsKeyPressed(KEY_ESCAPE))
            m_done = true;

        float wheel = GetMouseWheelMove();
        if (wheel != 0.0f) {
            m_scrollOffset -= (int)(wheel * ROW_H);
            m_scrollOffset = std::max(0, m_scrollOffset);
        }
    }

    void Render() override {
        int screenW = GetScreenWidth();
        int screenH = GetScreenHeight();

        ClearBackground({10, 10, 16, 255});

        int panelW = screenW * PANEL_W_PCT / 100;
        int panelH = screenH * PANEL_H_PCT / 100;
        int panelX = (screenW - panelW) / 2;
        int panelY = (screenH - panelH) / 2;

        DrawRectangle(panelX, panelY, panelW, panelH, {25, 25, 35, 255});
        DrawRectangleLinesEx({(float)panelX, (float)panelY, (float)panelW, (float)panelH}, 1, {70, 70, 100, 255});

        RenderHeader(panelX, panelY, panelW);

        int listAreaY = panelY + HEADER_H;
        int listAreaH = panelH - HEADER_H;
        int rowW = panelW - SCROLLBAR_W - 4;

        // Clamp scroll
        int totalH = (int)m_rows.size() * ROW_H;
        int maxScroll = std::max(0, totalH - listAreaH);
        m_scrollOffset = std::min(m_scrollOffset, maxScroll);

        Vector2 mouse = GetMousePosition();
        bool clicked = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

        BeginScissorMode(panelX, listAreaY, panelW, listAreaH);

        if (m_rows.empty()) {
            utils::DrawTextCentered(EmptyMessage(), panelX + panelW * 0.5f, listAreaY + listAreaH * 0.5f, 18, DARKGRAY);
        }

        for (int i = 0; i < (int)m_rows.size(); ++i) {
            int rowY = listAreaY + i * ROW_H - m_scrollOffset;
            if (rowY + ROW_H < listAreaY || rowY > listAreaY + listAreaH)
                continue;

            Rectangle rowRect = {(float)panelX, (float)rowY, (float)rowW, (float)ROW_H};
            int btnX = panelX + rowW - PADDING - BTN_W;
            int btnY = rowY + (ROW_H - BTN_H) / 2;
            Rectangle btnRect = {(float)btnX, (float)btnY, (float)BTN_W, (float)BTN_H};

            bool rowHov = CheckCollisionPointRec(mouse, rowRect);
            bool btnHov = CheckCollisionPointRec(mouse, btnRect);

            // Row background
            DrawRectangle(panelX, rowY, rowW, ROW_H, rowHov ? Color{50, 50, 70, 255} : Color{35, 35, 48, 255});
            DrawLine(panelX, rowY + ROW_H - 1, panelX + rowW, rowY + ROW_H - 1, {55, 55, 75, 255});

            // Delegate row content to subclass
            derived().RenderRow(m_rows[i], panelX, rowY, rowW, ROW_H, rowHov, btnHov);

            // Action button
            RenderActionButton(m_rows[i], btnRect, btnHov);

            if (btnHov && clicked && !derived().ActionDone(m_rows[i]))
                derived().OnRowAction(m_rows[i]);
        }

        EndScissorMode();
        RenderScrollbar(panelX, panelY, panelW, panelH, listAreaH);

        DrawText("ESC to close", panelX + PADDING, panelY + panelH - 20, 13, {90, 90, 110, 255});
    }

  protected:
    static constexpr int PANEL_W_PCT = 38;
    static constexpr int PANEL_H_PCT = 70;
    static constexpr int ROW_H = 52;
    static constexpr int HEADER_H = 56;
    static constexpr int BTN_W = 80;
    static constexpr int BTN_H = 30;
    static constexpr int PADDING = 16;
    static constexpr int SCROLLBAR_W = 6;

    std::vector<RowT> m_rows;
    bool m_done = false;
    int m_scrollOffset = 0;

    // Subclass hooks with defaults
    const char *EmptyMessage() const { return "No items found"; }

    Derived &derived() { return static_cast<Derived &>(*this); }
    const Derived &derived() const { return static_cast<const Derived &>(*this); }

  private:
    void RenderHeader(int panelX, int panelY, int panelW) {
        DrawRectangle(panelX, panelY, panelW, HEADER_H, {30, 30, 40, 255});
        DrawRectangleLinesEx({(float)panelX, (float)panelY, (float)panelW, (float)HEADER_H}, 1, {70, 70, 90, 255});

        utils::DrawTextCentered(derived().PanelTitle(), panelX + panelW * 0.5f, panelY + HEADER_H * 0.5f, 22, WHITE);

        Vector2 mouse = GetMousePosition();
        bool clicked = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
        int btnSz = 30;

        // Close button
        Rectangle closeBtn = {(float)(panelX + panelW - PADDING - btnSz),
                              (float)(panelY + (float)(HEADER_H - btnSz) / 2), (float)btnSz, (float)btnSz};
        bool closeHov = CheckCollisionPointRec(mouse, closeBtn);
        DrawRectangleRec(closeBtn, closeHov ? Color{180, 50, 50, 255} : Color{40, 40, 55, 255});
        DrawRectangleLinesEx(closeBtn, 1, {80, 80, 110, 255});
        utils::DrawTextCentered("X", closeBtn.x + closeBtn.width * 0.5f, closeBtn.y + closeBtn.height * 0.5f, 16,
                                LIGHTGRAY);
        if (closeHov && clicked)
            m_done = true;

        // Refresh button
        Rectangle refreshBtn = {(float)(panelX + panelW - PADDING - btnSz * 2 - 4),
                                (float)(panelY + (float)(HEADER_H - btnSz) / 2), (float)btnSz, (float)btnSz};
        bool refreshHov = CheckCollisionPointRec(mouse, refreshBtn);
        DrawRectangleRec(refreshBtn, refreshHov ? Color{60, 60, 80, 255} : Color{40, 40, 55, 255});
        DrawRectangleLinesEx(refreshBtn, 1, {80, 80, 110, 255});
        utils::DrawTextCentered("↺", refreshBtn.x + refreshBtn.width * 0.5f, refreshBtn.y + refreshBtn.height * 0.5f,
                                18, LIGHTGRAY);
        if (refreshHov && clicked) {
            m_rows.clear();
            m_scrollOffset = 0;
            derived().PopulateRows();
        }
    }

    void RenderActionButton(const RowT &row, Rectangle btnRect, bool hovered) {
        bool done = derived().ActionDone(row);
        if (done) {
            DrawRectangleRec(btnRect, {40, 80, 40, 255});
            DrawRectangleLinesEx(btnRect, 1, {80, 160, 80, 255});
            utils::DrawTextCentered(derived().ActionDoneLabel(row), btnRect.x + btnRect.width * 0.5f,
                                    btnRect.y + btnRect.height * 0.5f, 14, {120, 220, 120, 255});
        } else {
            DrawRectangleRec(btnRect, hovered ? Color{60, 110, 200, 255} : Color{40, 80, 160, 255});
            DrawRectangleLinesEx(btnRect, 1, {80, 140, 255, 255});
            utils::DrawTextCentered(derived().ActionLabel(row), btnRect.x + btnRect.width * 0.5f,
                                    btnRect.y + btnRect.height * 0.5f, 14, WHITE);
        }
    }

    void RenderScrollbar(int panelX, int panelY, int panelW, int panelH, int listAreaH) {
        int totalH = (int)m_rows.size() * ROW_H;
        if (totalH <= listAreaH)
            return;
        float ratio = (float)listAreaH / (float)totalH;
        int barH = std::max(30, (int)(listAreaH * ratio));
        float scrollFrac = (float)m_scrollOffset / (float)(totalH - listAreaH);
        int barY = panelY + HEADER_H + (int)(scrollFrac * (listAreaH - barH));
        DrawRectangle(panelX + panelW - SCROLLBAR_W - 2, barY, SCROLLBAR_W, barH, {90, 90, 120, 200});
    }
};

} // namespace UI
