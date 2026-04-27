#pragma once
#include "../../config.hpp"
#include "../../network/packets/lobby_packets.hpp"
#include "../../network/packets/packet_header.hpp"
#include "ITransport.hpp"
#include "steam/isteamuser.h"
#include "steam_transport.hpp"
#include <functional>
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
};

class SteamLobbyManager {
  public:
    explicit SteamLobbyManager(network::SteamTransport &transport) : m_transport(transport) {}

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
        SteamAPICall_t call = SteamMatchmaking()->JoinLobby(lobbyId);
        m_lobbyEnter.Set(call, this, &SteamLobbyManager::OnLobbyEnter);
    }

    // Opens the Steam overlay invite dialog for the host
    void OpenInviteDialog() {
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

    std::string GetLocalPlayerName() const { return SteamFriends()->GetPersonaName(); }

    network::PeerId AddHostToLobby() {
        CSteamID host = SteamMatchmaking()->GetLobbyOwner(m_lobbyId);
        return m_transport.RegisterPeer(host);
    }

  private:
    // ---- Steam callbacks ----

    void OnLobbyCreated(LobbyCreated_t *result, bool bIOFailure) {
        std::cout << "In the lobby created callback" << std::endl;
        if (bIOFailure || result->m_eResult != k_EResultOK) {
            if (m_callbacks.onError)
                m_callbacks.onError("Failed to create lobby");
            return;
        }
        m_lobbyId = result->m_ulSteamIDLobby;

        // Register the host as PEER_SERVER (PeerId 0)
        CSteamID hostId = SteamMatchmaking()->GetLobbyOwner(m_lobbyId);
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
        m_lobbyId = result->m_ulSteamIDLobby;
        CSteamID id = SteamUser()->GetSteamID();
        m_transport.RegisterPeer(id);

        if (m_callbacks.onLobbyJoined)
            m_callbacks.onLobbyJoined();

        network::JoinLobbyPacket pkt{};
        pkt.header.type = network::PacketType::JoinLobby;
        strncpy(pkt.name, GetLocalPlayerName().c_str(), sizeof(pkt.name) - 1);
        m_transport.send(network::PEER_SERVER, &pkt, sizeof(pkt));
    }

    // Member joined the lobby
    STEAM_CALLBACK(SteamLobbyManager, OnLobbyChatUpdate, LobbyChatUpdate_t) {
        if (pParam->m_ulSteamIDLobby != m_lobbyId.ConvertToUint64())
            return;

        CSteamID who = pParam->m_ulSteamIDUserChanged;
        uint32 change = pParam->m_rgfChatMemberStateChange;

        if (change & k_EChatMemberStateChangeEntered) {
            m_transport.RegisterPeer(who);
            if (m_callbacks.onMemberJoined)
                m_callbacks.onMemberJoined(who);
            if (GetMemberCount() >= MAX_PLAYERS && m_callbacks.onLobbyFull)
                m_callbacks.onLobbyFull();
        }
        if (change & (k_EChatMemberStateChangeLeft | k_EChatMemberStateChangeDisconnected)) {
            m_transport.CloseSession(who);
            if (m_callbacks.onMemberLeft)
                m_callbacks.onMemberLeft(who);
        }
    }

    // Handle friend clicking "Join Game" from Steam overlay
    STEAM_CALLBACK(SteamLobbyManager, OnGameLobbyJoinRequested, GameLobbyJoinRequested_t) {
        JoinLobby(pParam->m_steamIDLobby);
    }

    STEAM_CALLBACK(SteamLobbyManager, OnSessionRequest, SteamNetworkingMessagesSessionRequest_t) {
        CSteamID requester = pParam->m_identityRemote.GetSteamID();

        std::cout << "Session request from: " << requester.ConvertToUint64() << std::endl;

        // Only accept if you want to allow communication
        m_transport.AcceptSession(requester);
    }

    network::SteamTransport &m_transport;
    CSteamID m_lobbyId{};
    LobbyCallbacks m_callbacks{};

    CCallResult<SteamLobbyManager, LobbyCreated_t> m_lobbyCreated;
    CCallResult<SteamLobbyManager, LobbyEnter_t> m_lobbyEnter;
};
