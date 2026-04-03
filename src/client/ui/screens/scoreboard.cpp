#include "scoreboard.hpp"
#include "state/player_state.hpp"
#include <algorithm>
#include <string.h>
#include <string>

namespace UI {

static const int ROW_H = 46;
static const int HEADER_H = 34;
static const int TITLE_H = 36;
static const float PANEL_W = 480.0f;
static const float COL_RANK_X = 30.0f;
static const float COL_NAME_X = 60.0f;
static const float COL_KILLS_X = 320.0f;
static const float COL_DEATHS_X = 440.0f;

// Colors
static const Color C_PANEL_BG = {26, 26, 46, 240};
static const Color C_TITLE_BG = {22, 33, 62, 255};
static const Color C_HEADER_BG = {15, 52, 96, 255};
static const Color C_ROW_EVEN = {26, 26, 46, 255};
static const Color C_ROW_FIRST = {26, 42, 26, 255};
static const Color C_DIVIDER = {42, 42, 42, 255};
static const Color C_TEXT_HEAD = {170, 170, 204, 255};
static const Color C_TEXT_MAIN = {255, 255, 255, 255};
static const Color C_TEXT_DIM = {204, 204, 204, 255};
static const Color C_TEXT_RANK1 = {255, 204, 0, 255};
static const Color C_TEXT_RANK = {136, 136, 136, 255};
static const Color C_KILLS = {76, 255, 114, 255};
static const Color C_DEATHS = {255, 102, 102, 255};

Scoreboard::Scoreboard(const std::array<state::PlayerState, MAX_PLAYERS> &players) : m_players(players) {}

void Scoreboard::Render() {

    state::PlayerState sorted[MAX_PLAYERS];
    SortPlayerArray(sorted);

    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();

    float totalH = (float)(TITLE_H + HEADER_H + ROW_H * MAX_PLAYERS);
    float panelX = (screenW - PANEL_W) * 0.5f;
    float panelY = (screenH - totalH) * 0.5f;

    DrawPanelBackground(totalH, panelX, panelY);
    DrawTitleBar(panelX, panelY);

    float headerY = panelY + TITLE_H;
    float divY = headerY + HEADER_H;
    DrawHeaderRow(panelX, panelY, headerY, divY);

    DrawPlayerRows(panelX, divY, sorted);

    DrawVerticalColDividers(panelX, panelY, totalH);
}

void Scoreboard::SortPlayerArray(state::PlayerState *sorted) {
    memcpy(sorted, &m_players, sizeof(state::PlayerState) * MAX_PLAYERS);
    std::sort(sorted, sorted + MAX_PLAYERS,
              [](const state::PlayerState &a, const state::PlayerState &b) { return a.score.kills > b.score.kills; });
}

void Scoreboard::DrawTextCentered(const char *text, float cx, float y, int fontSize, Color color) {
    int w = MeasureText(text, fontSize);
    DrawText(text, (int)(cx - w * 0.5f), (int)y, fontSize, color);
}

void Scoreboard::DrawPanelBackground(float totalH, float panelX, float panelY) {
    DrawRectangleRounded({panelX, panelY, PANEL_W, totalH}, 0.04f, 8, C_PANEL_BG);
    DrawRectangleRoundedLines({panelX, panelY, PANEL_W, totalH}, 0.04f, 8, C_DIVIDER);
}

void Scoreboard::DrawTitleBar(float panelX, float panelY) {
    DrawRectangleRounded({panelX, panelY, PANEL_W, (float)TITLE_H}, 0.08f, 8, C_TITLE_BG);
    DrawRectangle((int)panelX, (int)(panelY + TITLE_H * 0.5f), (int)PANEL_W, TITLE_H / 2, C_TITLE_BG); // flatten bottom
    DrawTextCentered("SCOREBOARD", panelX + PANEL_W * 0.5f, panelY + (TITLE_H - 14) * 0.5f, 14, C_TEXT_MAIN);
}

void Scoreboard::DrawHeaderRow(float panelX, float panelY, float headerY, float divY) {
    DrawRectangle((int)panelX, (int)headerY, (int)PANEL_W, HEADER_H, C_HEADER_BG);
    DrawText("PLAYER", (int)(panelX + COL_NAME_X), (int)(headerY + 10), 12, C_TEXT_HEAD);
    DrawTextCentered("KILLS", panelX + COL_KILLS_X, headerY + 10, 12, C_TEXT_HEAD);
    DrawTextCentered("DEATHS", panelX + COL_DEATHS_X, headerY + 10, 12, C_TEXT_HEAD);
    DrawLine((int)panelX, (int)divY, (int)(panelX + PANEL_W), (int)divY, C_DIVIDER);
}

void Scoreboard::DrawPlayerRows(float panelX, float divY, state::PlayerState *sorted) {
    for (int i = 0; i < MAX_PLAYERS; i++) {
        float rowY = divY + i * ROW_H;
        Color rowBg = (i == 0) ? C_ROW_FIRST : C_ROW_EVEN;
        DrawRectangle((int)panelX, (int)rowY, (int)PANEL_W, ROW_H, rowBg);

        float textY = rowY + (ROW_H - 13) * 0.5f;

        if (sorted[i].active) {
            // Rank number
            Color rankColor = (i == 0) ? C_TEXT_RANK1 : C_TEXT_RANK;
            DrawText(TextFormat("%d", i + 1), (int)(panelX + COL_RANK_X), (int)textY, 11, rankColor);

            // Name
            Color nameColor = (i == 0) ? C_TEXT_MAIN : C_TEXT_DIM;
            // TODO: Update this to be the players name
            DrawText(std::to_string(sorted[i].id).c_str(), (int)(panelX + COL_NAME_X), (int)textY, 13, nameColor);

            // Kills / Deaths
            DrawTextCentered(TextFormat("%d", sorted[i].score.kills), panelX + COL_KILLS_X, textY, 13, C_KILLS);
            DrawTextCentered(TextFormat("%d", sorted[i].score.deaths), panelX + COL_DEATHS_X, textY, 13, C_DEATHS);
        } else {
            // Empty slot
            DrawText("-", (int)(panelX + COL_NAME_X), (int)textY, 13, C_TEXT_RANK);
        }

        // Row divider
        if (i < MAX_PLAYERS - 1) {
            float lineY = rowY + ROW_H;
            DrawLine((int)panelX, (int)lineY, (int)(panelX + PANEL_W), (int)lineY, C_DIVIDER);
        }
    }
}

void Scoreboard::DrawVerticalColDividers(float panelX, float panelY, float totalH) {
    float colDiv1 = panelX + COL_KILLS_X - 60;
    float colDiv2 = panelX + COL_DEATHS_X - 60;
    float colTop = panelY + TITLE_H;
    float colBot = panelY + totalH;
    DrawLine((int)colDiv1, (int)colTop, (int)colDiv1, (int)colBot, C_DIVIDER);
    DrawLine((int)colDiv2, (int)colTop, (int)colDiv2, (int)colBot, C_DIVIDER);
}

}; // namespace UI
