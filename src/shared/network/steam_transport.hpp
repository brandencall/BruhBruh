#pragma once
#include "../../network/packets/packet_header.hpp"
#include "ITransport.hpp"
#include <algorithm>
#include <atomic>
#include <cstddef>
#include <mutex>
#include <queue>
#include <shared_mutex>
#include <steam/isteamuser.h>
#include <steam/steam_api.h>
#include <unordered_map>

namespace network {

struct ThreadSafeQueue {
    void push(const InboundPacket &pkt) {
        std::lock_guard lk(m_mtx);
        m_packets.push(pkt);
    }

    bool pop(InboundPacket &out) {
        std::lock_guard lk(m_mtx);
        if (m_packets.empty())
            return false;
        out = m_packets.front();
        m_packets.pop();
        return true;
    }

  private:
    std::mutex m_mtx;
    std::queue<InboundPacket> m_packets;
};

// Maps our internal PeerId to/from CSteamID.
// Used by both the host (acts as "server") and clients.
class SteamTransport : public ITransport {
  public:
    SteamTransport() : m_localSteamId(SteamUser()->GetSteamID()) {}
    ~SteamTransport() override {};

    // ---- ITransport ----

    bool send(PeerId to, const void *data, size_t size) override {
        std::shared_lock lk(m_mapMtx);
        if (!m_running)
            return false;
        auto it = m_peerToSteam.find(to);
        if (it == m_peerToSteam.end())
            return false;

        if (it->second == m_localSteamId) {
            InboundPacket pkt;
            size_t sz = std::min<size_t>(size, MAX_PACKET_SIZE);
            memcpy(pkt.data, data, sz);
            pkt.size = sz;

            if (to == PEER_SERVER) {
                pkt.from = m_steamToPeer.at(SteamUser()->GetSteamID().ConvertToUint64());
                m_serverQueue.push(pkt);
            } else {
                pkt.from = PEER_SERVER;
                m_clientQueue.push(pkt);
            }
            return true;
        }

        SteamNetworkingIdentity identity{};
        identity.SetSteamID(it->second);

        auto *header = reinterpret_cast<const network::PacketHeader *>(data);
        bool reliable = IsReliablePacket(header->type);
        int sendFlags = reliable ? k_nSteamNetworkingSend_Reliable : k_nSteamNetworkingSend_Unreliable;
        int channel = reliable ? 1 : 0;

        auto result =
            SteamNetworkingMessages()->SendMessageToUser(identity, data, static_cast<uint32>(size), sendFlags, channel);
        return result == k_EResultOK;
    }

    void Pump() override {
        if (!m_running)
            return;

        for (int channel : {0, 1}) {
            SteamNetworkingMessage_t *msg = nullptr;
            while (SteamNetworkingMessages()->ReceiveMessagesOnChannel(channel, &msg, 1) > 0) {
                InboundPacket pkt;
                CSteamID sender = msg->m_identityPeer.GetSteamID();
                size_t sz = std::min<size_t>(msg->GetSize(), MAX_PACKET_SIZE);
                memcpy(pkt.data, msg->GetData(), sz);
                pkt.size = sz;
                msg->Release();

                {
                    std::unique_lock lk(m_mapMtx);
                    pkt.from = getOrRegisterPeer(sender);
                }

                if (pkt.from == PEER_SERVER)
                    m_clientQueue.push(pkt);
                else
                    m_serverQueue.push(pkt);
            }
        }
    }

    // recv() becomes just a queue pop — no Steam calls, safe from any thread:
    bool recvServer(InboundPacket &out) override { return m_serverQueue.pop(out); }
    bool recvClient(InboundPacket &out) override { return m_clientQueue.pop(out); }

    // ---- Peer management ----

    // Called by SteamLobbyManager once lobby members are known.
    // Host calls this for each client; clients call it for the host.
    PeerId RegisterPeer(CSteamID steamId) { return getOrRegisterPeer(steamId); }

    // In SteamTransport — add this public method:
    void RegisterPeerAs(CSteamID steamId, PeerId forcedId) {
        m_peerToSteam[forcedId] = steamId;
        m_steamToPeer[steamId.ConvertToUint64()] = forcedId;
        // Ensure nextPeerId doesn't collide
        if (m_nextPeerId <= forcedId)
            m_nextPeerId = forcedId + 1;
    }

    CSteamID GetSteamID(PeerId peer) const {
        auto it = m_peerToSteam.find(peer);
        if (it == m_peerToSteam.end())
            return CSteamID{};
        return it->second;
    }

    // Accept incoming session requests (called from SteamLobbyManager callback)
    void AcceptSession(CSteamID from) {
        SteamNetworkingIdentity identity{};
        identity.SetSteamID(from);
        bool result = SteamNetworkingMessages()->AcceptSessionWithUser(identity);
    }

    void CloseSession(CSteamID from) {
        SteamNetworkingIdentity identity{};
        identity.SetSteamID(from);
        SteamNetworkingMessages()->CloseSessionWithUser(identity);
    }

    void CloseAllSessions() {
        std::unique_lock lk(m_mapMtx);
        for (auto &[peerId, steamId] : m_peerToSteam) {
            SteamNetworkingIdentity identity{};
            identity.SetSteamID(steamId);
            SteamNetworkingMessages()->CloseSessionWithUser(identity);
        }
        m_peerToSteam.clear();
        m_steamToPeer.clear();
    }

    void Shutdown() {
        CloseAllSessions();
        m_running = false;
    }

  private:
    PeerId getOrRegisterPeer(CSteamID id) {
        auto it = m_steamToPeer.find(id.ConvertToUint64());
        if (it != m_steamToPeer.end() && it->second != network::PEER_SERVER)
            return it->second;

        PeerId newId = m_nextPeerId++;
        m_peerToSteam[newId] = id;
        m_steamToPeer[id.ConvertToUint64()] = newId;
        return newId;
    }

    uint32_t m_nextPeerId = 1; // 0 is PEER_SERVER

    std::shared_mutex m_mapMtx;
    std::unordered_map<PeerId, CSteamID> m_peerToSteam;
    std::unordered_map<uint64, PeerId> m_steamToPeer;
    std::atomic<bool> m_running = true;
    ThreadSafeQueue m_serverQueue;
    ThreadSafeQueue m_clientQueue;
    CSteamID m_localSteamId;
};

} // namespace network
