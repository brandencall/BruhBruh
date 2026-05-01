#include "friends_invite_screen.hpp"

namespace UI {

FriendsInviteScreen::FriendsInviteScreen(SteamLobbyManager &lobbyManager) : m_lobbyManager(lobbyManager) {
    PopulateRows();
}

// ── SteamListPanel hooks ──────────────────────────────────────────────

const char *FriendsInviteScreen::PanelTitle() const { return "Invite Friends"; }
const char *FriendsInviteScreen::EmptyMessage() const { return "No online friends found"; }

void FriendsInviteScreen::PopulateRows() {
    int count = SteamFriends()->GetFriendCount(k_EFriendFlagImmediate);
    for (int i = 0; i < count; ++i) {
        CSteamID id = SteamFriends()->GetFriendByIndex(i, k_EFriendFlagImmediate);
        EPersonaState state = SteamFriends()->GetFriendPersonaState(id);
        if (state == k_EPersonaStateOffline)
            continue;
        m_rows.push_back({id, SteamFriends()->GetFriendPersonaName(id), state});
    }
    std::sort(m_rows.begin(), m_rows.end(), [](const FriendEntry &a, const FriendEntry &b) {
        if (a.state != b.state)
            return a.state > b.state;
        return a.name < b.name;
    });
}

void FriendsInviteScreen::RenderRow(const FriendEntry &e, int x, int y, int /*rowW*/, int rowH, bool /*rowHov*/,
                                    bool /*btnHov*/) const {
    DrawRowIdentity(x, y, rowH, PADDING, e.name, e.state);
}

void FriendsInviteScreen::OnRowAction(FriendEntry &e) {
    m_lobbyManager.InviteFriend(e.steamId);
    e.invited = true;
}
bool FriendsInviteScreen::ActionDone(const FriendEntry &e) const { return e.invited; }
const char *FriendsInviteScreen::ActionLabel(const FriendEntry &) const { return "Invite"; }
const char *FriendsInviteScreen::ActionDoneLabel(const FriendEntry &) const { return "Invited"; }

} // namespace UI
