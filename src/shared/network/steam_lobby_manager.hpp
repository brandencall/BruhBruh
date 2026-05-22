#pragma once
#include "../../config.hpp"
#include "ITransport.hpp"
#include "steam/isteammatchmaking.h"
#include "steam_transport.hpp"
#include <functional>
#include <iostream>
#include <steam/isteamuser.h>
#include <steam/steam_api.h>
#include <string>
#include <vector>

// Callbacks the game acts on
struct LobbyCallbacks {
    std::function<void()> onLobbyCreated;         // Host: lobby is ready
    std::function<void()> onLobbyJoined;          // Client: joined successfully
    std::function<void(CSteamID)> onMemberJoined; // Someone else joined
    std::function<void(CSteamID)> onMemberLeft;   // Someone left
    std::function<void()> onLobbyFull;
    std::function<void(const char *)> onError;
    std::function<void(CSteamID, CSteamID)> onInviteAccepted; // (fromId, lobbyId)
    std::function<void(CSteamID)> onJoinRequested;            // rich presence join
};

class SteamLobbyManager {
  public:
    explicit SteamLobbyManager(network::SteamTransport &transport) : m_transport(transport) {}

    SteamLobbyManager(const SteamLobbyManager &) = delete;
    SteamLobbyManager &operator=(const SteamLobbyManager &) = delete;
    SteamLobbyManager(SteamLobbyManager &&) = delete;
    SteamLobbyManager &operator=(SteamLobbyManager &&) = delete;

    ~SteamLobbyManager() {
        m_lobbyCreated.Cancel();
        m_lobbyEnter.Cancel();
    }

    void SetCallbacks(LobbyCallbacks cb) { m_callbacks = std::move(cb); }

    // ---- Host flow ----
    void CreateLobby(int maxPlayers = MAX_PLAYERS) {
        SteamAPICall_t call = SteamMatchmaking()->CreateLobby(k_ELobbyTypeFriendsOnly, maxPlayers);
        m_lobbyCreated.Set(call, this, &SteamLobbyManager::OnLobbyCreated);
    }

    // ---- Client flow ----
    void JoinLobby(CSteamID lobbyId) {
        if (!lobbyId.IsValid())
            return;
        m_lobbyId = lobbyId;
        SteamAPICall_t call = SteamMatchmaking()->JoinLobby(lobbyId);
        m_lobbyEnter.Set(call, this, &SteamLobbyManager::OnLobbyEnter);
    }

    // Opens the Steam overlay invite dialog for the host
    void OpenInviteDialog() {
        if (!SteamUtils()->IsOverlayEnabled()) {
            std::cout << "Steam overlay is not enabled" << std::endl;
        }
        if (m_lobbyId.IsValid())
            SteamFriends()->ActivateGameOverlayInviteDialog(m_lobbyId);
    }

    // ---- State ----
    CSteamID GetLobbyId() const { return m_lobbyId; }
    bool IsValid() const { return m_lobbyId.IsValid(); }

    int GetMemberCount() const {
        if (!m_lobbyId.IsValid())
            return 0;
        return SteamMatchmaking()->GetNumLobbyMembers(m_lobbyId);
    }

    // Returns SteamIDs of all members except local player
    std::vector<CSteamID> GetRemoteMembers() const {
        std::vector<CSteamID> result;
        if (!m_lobbyId.IsValid())
            return result;
        int count = SteamMatchmaking()->GetNumLobbyMembers(m_lobbyId);
        CSteamID localId = SteamUser()->GetSteamID();
        for (int i = 0; i < count; ++i) {
            CSteamID member = SteamMatchmaking()->GetLobbyMemberByIndex(m_lobbyId, i);
            if (member != localId)
                result.push_back(member);
        }
        return result;
    }

    bool IsLocalPlayerHost() const {
        if (!m_lobbyId.IsValid())
            return false;
        return SteamMatchmaking()->GetLobbyOwner(m_lobbyId) == SteamUser()->GetSteamID();
    }

    void LeaveLobby() {
        if (m_lobbyId.IsValid()) {
            SteamMatchmaking()->LeaveLobby(m_lobbyId);
            m_lobbyId = CSteamID{};
        }
    }

    std::vector<CSteamID> GetOnlineFriends() {
        std::vector<CSteamID> friends;
        int count = SteamFriends()->GetFriendCount(k_EFriendFlagImmediate);
        for (int i = 0; i < count; ++i) {
            CSteamID id = SteamFriends()->GetFriendByIndex(i, k_EFriendFlagImmediate);
            if (SteamFriends()->GetFriendPersonaState(id) != k_EPersonaStateOffline) {
                friends.push_back(id);
            }
        }
        return friends;
    }

    void InviteFriend(CSteamID friendId) { SteamMatchmaking()->InviteUserToLobby(m_lobbyId, friendId); }

    std::string GetLocalPlayerName() const { return SteamFriends()->GetPersonaName(); }

    network::PeerId AddHostToLobby() {
        CSteamID host = SteamMatchmaking()->GetLobbyOwner(m_lobbyId);
        // Force a fresh non-zero PeerId for the host as a player,
        // independent of the PEER_SERVER mapping
        return m_transport.RegisterHostPlayer(host);
    }

  private:
    // ---- Steam callbacks ----

    void OnLobbyCreated(LobbyCreated_t *result, bool bIOFailure) {
        if (bIOFailure || result->m_eResult != k_EResultOK) {
            if (m_callbacks.onError)
                m_callbacks.onError("Failed to create lobby");
            return;
        }
        m_lobbyId = result->m_ulSteamIDLobby;
        CSteamID hostId = SteamMatchmaking()->GetLobbyOwner(m_lobbyId);

        SteamMatchmaking()->SetLobbyType(m_lobbyId, k_ELobbyTypePublic);
        SteamMatchmaking()->SetLobbyData(m_lobbyId, "game", "BruhBruh");
        SteamMatchmaking()->SetLobbyData(m_lobbyId, "host_name", SteamFriends()->GetPersonaName());
        SteamMatchmaking()->SetLobbyData(m_lobbyId, "host_id", std::to_string(hostId.ConvertToUint64()).c_str());

        std::string connectStr = std::to_string(m_lobbyId.ConvertToUint64());
        SteamFriends()->SetRichPresence("connect", connectStr.c_str());
        SteamFriends()->SetRichPresence("steam_player_group", connectStr.c_str());
        SteamFriends()->SetRichPresence("steam_player_group_size", "1");
        SteamFriends()->SetRichPresence("status", "In Lobby");

        // Register the host as PEER_SERVER (PeerId 0)
        m_transport.RegisterPeerAs(hostId, network::PEER_SERVER);

        if (m_callbacks.onLobbyCreated)
            m_callbacks.onLobbyCreated();
    }

    void OnLobbyEnter(LobbyEnter_t *result, bool bIOFailure) {
        if (bIOFailure || result->m_EChatRoomEnterResponse != k_EChatRoomEnterResponseSuccess) {
            if (m_callbacks.onError)
                m_callbacks.onError("Failed to join lobby");
            return;
        }

        CSteamID hostId = SteamMatchmaking()->GetLobbyOwner(m_lobbyId);
        m_transport.RegisterPeerAs(hostId, network::PEER_SERVER);

        if (m_callbacks.onLobbyJoined)
            m_callbacks.onLobbyJoined();
    }

    STEAM_CALLBACK(SteamLobbyManager, OnLobbyChatUpdate, LobbyChatUpdate_t) {
        std::cout << "In the lobby changed callback" << std::endl;
        std::cout << "The param is: " << pParam->m_rgfChatMemberStateChange << std::endl;
        if (pParam->m_rgfChatMemberStateChange & k_EChatMemberStateChangeDisconnected ||
            pParam->m_rgfChatMemberStateChange & k_EChatMemberStateChangeLeft) {
            std::cout << "Detected the member leaving" << std::endl;
        }
    }

    STEAM_CALLBACK(SteamLobbyManager, OnLobbyInvite, LobbyInvite_t) {
        std::cout << "Lobby invite received: " << pParam->m_ulSteamIDLobby << "\n";

        if (m_callbacks.onInviteAccepted) {
            m_callbacks.onInviteAccepted(CSteamID(pParam->m_ulSteamIDUser), CSteamID(pParam->m_ulSteamIDLobby));
        }
    }

    STEAM_CALLBACK(SteamLobbyManager, OnSessionRequest, SteamNetworkingMessagesSessionRequest_t) {
        CSteamID requester = pParam->m_identityRemote.GetSteamID();
        // Only accept if you want to allow communication
        m_transport.AcceptSession(requester);
    }

    network::SteamTransport &m_transport;
    CSteamID m_lobbyId{};
    LobbyCallbacks m_callbacks{};

    CCallResult<SteamLobbyManager, LobbyCreated_t> m_lobbyCreated;
    CCallResult<SteamLobbyManager, LobbyEnter_t> m_lobbyEnter;
};
