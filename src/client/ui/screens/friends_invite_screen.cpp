#include "friends_invite_screen.hpp"
#include "../../utils/text_utils.hpp"
#include "raylib.h"
#include <algorithm>
#include <steam/steam_api.h>

namespace UI {

// ─────────────────────────────────────────────────────────────────────────────
// Construction
// ─────────────────────────────────────────────────────────────────────────────

FriendsInviteScreen::FriendsInviteScreen(SteamLobbyManager &lobbyManager) : m_lobbyManager(lobbyManager) {
    RefreshFriends();
}

// ─────────────────────────────────────────────────────────────────────────────
// Friends list population
// ─────────────────────────────────────────────────────────────────────────────

void FriendsInviteScreen::RefreshFriends() {
    m_friends.clear();

    int count = SteamFriends()->GetFriendCount(k_EFriendFlagImmediate);
    for (int i = 0; i < count; ++i) {
        CSteamID id = SteamFriends()->GetFriendByIndex(i, k_EFriendFlagImmediate);
        EPersonaState state = SteamFriends()->GetFriendPersonaState(id);
        if (state == k_EPersonaStateOffline)
            continue;

        FriendEntry entry{};
        entry.steamId = id;
        entry.name = SteamFriends()->GetFriendPersonaName(id);
        entry.state = state;
        m_friends.push_back(entry);
    }

    // Sort: online first, then by name
    std::sort(m_friends.begin(), m_friends.end(), [](const FriendEntry &a, const FriendEntry &b) {
        if (a.state != b.state)
            return a.state > b.state; // higher state value = more "online"
        return a.name < b.name;
    });
}

// ─────────────────────────────────────────────────────────────────────────────
// Update
// ─────────────────────────────────────────────────────────────────────────────

void FriendsInviteScreen::Update(float dt) {
    // Close on Escape
    if (IsKeyPressed(KEY_ESCAPE))
        m_done = true;

    // Mouse wheel scrolling
    float wheel = GetMouseWheelMove();
    if (wheel != 0.0f) {
        m_scrollOffset -= (int)(wheel * ROW_H);
        m_scrollOffset = std::max(0, m_scrollOffset);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Render helpers
// ─────────────────────────────────────────────────────────────────────────────

Color FriendsInviteScreen::GetStateColor(EPersonaState state) const {
    switch (state) {
    case k_EPersonaStateOnline:
        return {100, 220, 120, 255}; // green
    case k_EPersonaStateBusy:
        return {220, 100, 100, 255}; // red
    case k_EPersonaStateAway:
    case k_EPersonaStateSnooze:
        return {200, 180, 60, 255}; // yellow
    default:
        return {150, 150, 150, 255}; // grey
    }
}

const char *FriendsInviteScreen::GetStateLabel(EPersonaState state) const {
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

void FriendsInviteScreen::RenderBackground(int screenW, int screenH) {
    DrawRectangle(0, 0, screenW, screenH, {0, 0, 0, 255});
}

void FriendsInviteScreen::RenderHeader(int panelX, int panelY, int panelW) {
    // Title bar background
    DrawRectangle(panelX, panelY, panelW, HEADER_H, {30, 30, 40, 255});
    DrawRectangleLinesEx({(float)panelX, (float)panelY, (float)panelW, (float)HEADER_H}, 1, {70, 70, 90, 255});

    utils::DrawTextCentered("Invite Friends", panelX + panelW * 0.5f, panelY + HEADER_H * 0.5f, 22, WHITE);

    // Refresh button (top-right)
    int btnSize = 30;
    Rectangle refreshBtn = {(float)(panelX + panelW - PADDING - btnSize),
                            (float)(panelY + (float)(HEADER_H - btnSize) / 2), (float)btnSize, (float)btnSize};
    Vector2 mouse = GetMousePosition();
    bool refreshHovered = CheckCollisionPointRec(mouse, refreshBtn);
    DrawRectangleRec(refreshBtn, refreshHovered ? Color{60, 60, 80, 255} : Color{40, 40, 55, 255});
    DrawRectangleLinesEx(refreshBtn, 1, {80, 80, 110, 255});
    utils::DrawTextCentered("↺", refreshBtn.x + refreshBtn.width * 0.5f, refreshBtn.y + refreshBtn.height * 0.5f, 18,
                            LIGHTGRAY);

    if (refreshHovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        RefreshFriends();

    // Close button (top-right, after refresh)
    Rectangle closeBtn = {(float)(panelX + panelW - PADDING - btnSize * 2 - 4),
                          (float)(panelY + (float)(HEADER_H - btnSize) / 2), (float)btnSize, (float)btnSize};
    bool closeHovered = CheckCollisionPointRec(mouse, closeBtn);
    DrawRectangleRec(closeBtn, closeHovered ? Color{180, 50, 50, 255} : Color{40, 40, 55, 255});
    DrawRectangleLinesEx(closeBtn, 1, {80, 80, 110, 255});
    utils::DrawTextCentered("X", closeBtn.x + closeBtn.width * 0.5f, closeBtn.y + closeBtn.height * 0.5f, 16,
                            LIGHTGRAY);

    if (closeHovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        m_done = true;
}

void FriendsInviteScreen::RenderFriendRow(const FriendEntry &entry, int x, int y, int rowW, int rowH, bool hovered,
                                          bool inviteHovered) {
    // Row background
    Color rowBg = hovered ? Color{50, 50, 70, 255} : Color{35, 35, 48, 255};
    DrawRectangle(x, y, rowW, rowH, rowBg);
    DrawLine(x, y + rowH - 1, x + rowW, y + rowH - 1, {55, 55, 75, 255});

    // Status dot
    int dotR = 6;
    int dotX = x + PADDING + dotR;
    int dotY = y + rowH / 2;
    DrawCircle(dotX, dotY, dotR, GetStateColor(entry.state));

    // Name
    int nameX = dotX + dotR + PADDING;
    int nameY = y + rowH / 2 - 10;
    DrawText(entry.name.c_str(), nameX, nameY, 18, WHITE);

    // Status label
    int statusFontSize = 13;
    DrawText(GetStateLabel(entry.state), nameX, nameY + 20, statusFontSize, GetStateColor(entry.state));

    // Invite / Invited button
    int btnX = x + rowW - PADDING - BTN_W;
    int btnY = y + (rowH - BTN_H) / 2;
    Rectangle btnRect = {(float)btnX, (float)btnY, (float)BTN_W, (float)BTN_H};

    if (entry.invited) {
        DrawRectangleRec(btnRect, {40, 80, 40, 255});
        DrawRectangleLinesEx(btnRect, 1, {80, 160, 80, 255});
        utils::DrawTextCentered("Invited", btnRect.x + btnRect.width * 0.5f, btnRect.y + btnRect.height * 0.5f, 14,
                                {120, 220, 120, 255});
    } else {
        Color btnColor = inviteHovered ? Color{60, 110, 200, 255} : Color{40, 80, 160, 255};
        DrawRectangleRec(btnRect, btnColor);
        DrawRectangleLinesEx(btnRect, 1, {80, 140, 255, 255});
        utils::DrawTextCentered("Invite", btnRect.x + btnRect.width * 0.5f, btnRect.y + btnRect.height * 0.5f, 14,
                                WHITE);
    }
}

void FriendsInviteScreen::RenderScrollbar(int panelX, int panelY, int panelW, int panelH, int listAreaH, int rowH) {
    int totalContentH = (int)m_friends.size() * rowH;
    if (totalContentH <= listAreaH)
        return;

    float ratio = (float)listAreaH / (float)totalContentH;
    int barH = std::max(30, (int)(listAreaH * ratio));
    float scrollFrac = (float)m_scrollOffset / (float)(totalContentH - listAreaH);
    int barY = panelY + HEADER_H + (int)(scrollFrac * (listAreaH - barH));

    DrawRectangle(panelX + panelW - SCROLLBAR_W - 2, barY, SCROLLBAR_W, barH, {90, 90, 120, 200});
}

// ─────────────────────────────────────────────────────────────────────────────
// Main Render
// ─────────────────────────────────────────────────────────────────────────────

void FriendsInviteScreen::Render() {
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();

    RenderBackground(screenW, screenH);

    // Panel dimensions
    int panelW = screenW * PANEL_W_PCT / 100;
    int panelH = screenH * PANEL_H_PCT / 100;
    int panelX = (screenW - panelW) / 2;
    int panelY = (screenH - panelH) / 2;

    // Panel background + border
    DrawRectangle(panelX, panelY, panelW, panelH, {25, 25, 35, 255});
    DrawRectangleLinesEx({(float)panelX, (float)panelY, (float)panelW, (float)panelH}, 1, {70, 70, 100, 255});

    RenderHeader(panelX, panelY, panelW);

    // List area
    int listAreaY = panelY + HEADER_H;
    int listAreaH = panelH - HEADER_H;
    int rowW = panelW - SCROLLBAR_W - 4;

    Vector2 mouse = GetMousePosition();
    bool mouseClicked = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

    // Clamp scroll
    int totalContentH = (int)m_friends.size() * ROW_H;
    int maxScroll = std::max(0, totalContentH - listAreaH);
    m_scrollOffset = std::min(m_scrollOffset, maxScroll);

    // Scissor to clip rows inside list area
    BeginScissorMode(panelX, listAreaY, panelW, listAreaH);

    if (m_friends.empty()) {
        utils::DrawTextCentered("No online friends found", panelX + panelW * 0.5f, listAreaY + listAreaH * 0.5f, 18,
                                DARKGRAY);
    }

    for (int i = 0; i < (int)m_friends.size(); ++i) {
        int rowY = listAreaY + i * ROW_H - m_scrollOffset;

        // Skip rows fully outside the panel
        if (rowY + ROW_H < listAreaY || rowY > listAreaY + listAreaH)
            continue;

        Rectangle rowRect = {(float)panelX, (float)rowY, (float)rowW, (float)ROW_H};

        int btnX = panelX + rowW - PADDING - BTN_W;
        int btnY = rowY + (ROW_H - BTN_H) / 2;
        Rectangle btnRect = {(float)btnX, (float)btnY, (float)BTN_W, (float)BTN_H};

        bool rowHovered = CheckCollisionPointRec(mouse, rowRect);
        bool inviteHovered = CheckCollisionPointRec(mouse, btnRect);

        RenderFriendRow(m_friends[i], panelX, rowY, rowW, ROW_H, rowHovered, inviteHovered);

        // Handle invite click
        if (!m_friends[i].invited && inviteHovered && mouseClicked) {
            m_lobbyManager.InviteFriend(m_friends[i].steamId);
            m_friends[i].invited = true;
        }
    }

    EndScissorMode();

    RenderScrollbar(panelX, panelY, panelW, panelH, listAreaH, ROW_H);

    // Footer hint
    DrawText("ESC to close", panelX + PADDING, panelY + panelH - 20, 13, {90, 90, 110, 255});
}

} // namespace UI
