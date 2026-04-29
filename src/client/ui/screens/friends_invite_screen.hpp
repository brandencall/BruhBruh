#pragma once
#include "../../../shared/network/steam_lobby_manager.hpp"
#include "../ui_manager.hpp"
#include "raylib.h"
#include <steam/steam_api.h>
#include <string>
#include <vector>

namespace UI {

struct FriendEntry {
    CSteamID steamId;
    std::string name;
    EPersonaState state;
    bool invited = false;
};

class FriendsInviteScreen : public UIScreen {
  public:
    explicit FriendsInviteScreen(SteamLobbyManager &lobbyManager);

    void Update(float dt) override;
    void Render() override;
    bool BlocksGameInput() const override { return true; }
    bool IsDone() const override { return m_done; }

  private:
    void RefreshFriends();
    void RenderBackground(int screenW, int screenH);
    void RenderPanel(int screenW, int screenH);
    void RenderHeader(int panelX, int panelY, int panelW);
    void RenderFriendRow(const FriendEntry &entry, int x, int y, int rowW, int rowH, bool hovered, bool inviteHovered);
    void RenderScrollbar(int panelX, int panelY, int panelW, int panelH, int listAreaH, int rowH);
    Color GetStateColor(EPersonaState state) const;
    const char *GetStateLabel(EPersonaState state) const;

  private:
    SteamLobbyManager &m_lobbyManager;
    std::vector<FriendEntry> m_friends;
    bool m_done = false;

    // Scroll
    int m_scrollOffset = 0;

    // Layout constants (computed in Render from screen size)
    static constexpr int PANEL_W_PCT = 38; // % of screen width
    static constexpr int PANEL_H_PCT = 70;
    static constexpr int ROW_H = 52;
    static constexpr int HEADER_H = 56;
    static constexpr int BTN_W = 80;
    static constexpr int BTN_H = 30;
    static constexpr int PADDING = 16;
    static constexpr int SCROLLBAR_W = 6;
};

} // namespace UI
