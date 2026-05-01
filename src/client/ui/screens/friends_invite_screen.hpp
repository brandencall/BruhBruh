#pragma once
#include "../../../shared/network/steam_lobby_manager.hpp"
#include "list_panel_screen.hpp"

namespace UI {

struct FriendEntry {
    CSteamID steamId;
    std::string name;
    EPersonaState state;
    bool invited = false;
};

class FriendsInviteScreen : public SteamListPanel<FriendsInviteScreen, FriendEntry> {
  public:
    explicit FriendsInviteScreen(SteamLobbyManager &lobbyManager);

    // ── SteamListPanel hooks ──────────────────────────────────────────────

    const char *PanelTitle() const;
    const char *EmptyMessage() const;

    void PopulateRows();

    void RenderRow(const FriendEntry &e, int x, int y, int /*rowW*/, int rowH, bool /*rowHov*/, bool /*btnHov*/) const;

    void OnRowAction(FriendEntry &e);
    bool ActionDone(const FriendEntry &e) const;
    const char *ActionLabel(const FriendEntry &) const;
    const char *ActionDoneLabel(const FriendEntry &) const;

  private:
    SteamLobbyManager &m_lobbyManager;
};

} // namespace UI
