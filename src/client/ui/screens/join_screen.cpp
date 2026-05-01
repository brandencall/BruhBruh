#include "join_screen.hpp"
#include "raylib.h"
#include "steam/isteamfriends.h"
#include "steam/isteammatchmaking.h"
#include <charconv>
#include <iostream>
#include <system_error>
#include <unordered_set>

namespace UI {

JoinScreen::JoinScreen(std::function<void(CSteamID)> onJoin) : m_onJoin(std::move(onJoin)) {
    std::cout << "JoinScreen constructor" << std::endl;
    PopulateRows();
}

void JoinScreen::PopulateRows() {
    m_rows.clear();
    SteamMatchmaking()->AddRequestLobbyListFilterSlotsAvailable(1);
    SteamMatchmaking()->AddRequestLobbyListStringFilter("game", "BruhBruh", k_ELobbyComparisonEqual);
    SteamAPICall_t call = SteamMatchmaking()->RequestLobbyList();
    m_lobbyMatchList.Set(call, this, &JoinScreen::OnLobbyMatchList);
}

void JoinScreen::OnLobbyMatchList(LobbyMatchList_t *pCallback, bool bIOFailure) {
    if (bIOFailure)
        return;

    for (uint32 i = 0; i < pCallback->m_nLobbiesMatching; ++i) {
        CSteamID lobbyId = SteamMatchmaking()->GetLobbyByIndex(i);
        int members = SteamMatchmaking()->GetNumLobbyMembers(lobbyId);
        int maxMem = SteamMatchmaking()->GetLobbyMemberLimit(lobbyId);

        CSteamID hostId = SteamMatchmaking()->GetLobbyOwner(lobbyId);
        std::string hostName = SteamFriends()->GetFriendPersonaName(hostId);
        EPersonaState hostState = SteamFriends()->GetFriendPersonaState(hostId);

        // Fall back to lobby metadata name if Steam doesn't know the persona
        if (!hostId.IsValid() || hostName.empty()) {
            const char *metaIdStr = SteamMatchmaking()->GetLobbyData(lobbyId, "host_id");
            uint64 hostRaw = std::stoull(metaIdStr);
            hostId = CSteamID(hostRaw);
            if (!hostId.IsValid())
                continue;

            const char *metaName = SteamMatchmaking()->GetLobbyData(lobbyId, "host_name");
            hostName = metaName ? metaName : "Unknown";
            hostState = SteamFriends()->GetFriendPersonaState(hostId);
        }

        if (SteamFriends()->GetFriendRelationship(hostId) == k_EFriendRelationshipFriend) {
            m_rows.push_back({lobbyId, hostName, hostState, members, maxMem});
        }
    }
}

void JoinScreen::RenderRow(const LobbyEntry &e, int x, int y, int rowW, int rowH, bool /*rowHov*/,
                           bool /*btnHov*/) const {
    DrawRowIdentity(x, y, rowH, PADDING, e.hostName, e.hostState);

    std::string countStr = std::to_string(e.memberCount) + "/" + std::to_string(e.maxMembers);
    int countW = MeasureText(countStr.c_str(), 16);
    int countX = x + rowW - PADDING - BTN_W - PADDING - countW;
    DrawText(countStr.c_str(), countX, y + rowH / 2 - 8, 16, {160, 160, 180, 255});
}

void JoinScreen::OnRowAction(LobbyEntry &e) {
    e.joining = true;
    m_onJoin(e.lobbyId);
}

bool JoinScreen::ActionDone(const LobbyEntry &e) const { return e.joining; }
const char *JoinScreen::ActionLabel(const LobbyEntry &) const { return "Join"; }
const char *JoinScreen::ActionDoneLabel(const LobbyEntry &) const { return "Joining..."; }

} // namespace UI
