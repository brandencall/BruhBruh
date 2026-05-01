#pragma once
#include "list_panel_screen.hpp"
#include <functional>

namespace UI {

struct LobbyEntry {
    CSteamID lobbyId;
    std::string hostName;
    EPersonaState hostState = k_EPersonaStateOffline;
    int memberCount = 0;
    int maxMembers = 0;
    bool joining = false;
};

class JoinScreen : public SteamListPanel<JoinScreen, LobbyEntry> {
  public:
    explicit JoinScreen(std::function<void(CSteamID)> onJoin);

    const char *PanelTitle() const { return "Join a Friend's Game"; }
    const char *EmptyMessage() const { return "No friend lobbies found"; }

    void PopulateRows();
    void RenderRow(const LobbyEntry &e, int x, int y, int rowW, int rowH, bool rowHov, bool btnHov) const;

    void OnRowAction(LobbyEntry &e);
    bool ActionDone(const LobbyEntry &e) const;
    const char *ActionLabel(const LobbyEntry &) const;
    const char *ActionDoneLabel(const LobbyEntry &) const;

  private:
    void OnLobbyMatchList(LobbyMatchList_t *pCallback, bool bIOFailure);

    std::function<void(CSteamID)> m_onJoin;
    CCallResult<JoinScreen, LobbyMatchList_t> m_lobbyMatchList;
};

} // namespace UI
