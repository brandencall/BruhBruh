#include "server_lobby.hpp"
#include "characters/character_types.hpp"
#include "network/ITransport.hpp"
#include <cstdint>
#include <string.h>
#include <string>

int ServerLobby::AddPlayer(network::PeerId peer, const char *name, uint32_t playerId) {
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (!m_slots[i].lobbySlot.occupied) {
            m_slots[i].peerId = peer;
            m_slots[i].lobbySlot.id = playerId;
            m_slots[i].lobbySlot.ready = false;
            m_slots[i].lobbySlot.occupied = true;
            strncpy(m_slots[i].lobbySlot.name, name, sizeof(m_slots[i].lobbySlot.name) - 1);
            m_slots[i].lobbySlot.name[sizeof(m_slots[i].lobbySlot.name) - 1] = '\0';
            return i;
        }
    }
    return -1; // full
}

int ServerLobby::AddPlayer(network::PeerId peer, uint32_t playerId) {
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (!m_slots[i].lobbySlot.occupied) {
            m_slots[i].peerId = peer;
            m_slots[i].lobbySlot.id = playerId;
            m_slots[i].lobbySlot.ready = false;
            m_slots[i].lobbySlot.occupied = true;
            std::string name = "Player[" + std::to_string(i) + "]";
            strncpy(m_slots[i].lobbySlot.name, name.c_str(), sizeof(m_slots[i].lobbySlot.name) - 1);
            return i;
        }
    }
    return -1; // full
}

void ServerLobby::RemovePlayer(network::PeerId peer) {
    for (auto &slot : m_slots) {
        if (slot.lobbySlot.occupied && slot.peerId == peer) {
            slot = LobbySlot{}; // zero it out
            return;
        }
    }
}

bool ServerLobby::TrySetCharacter(network::PeerId peer, Character::CharacterId characterId) {
    for (auto &slot : m_slots) {
        if (slot.lobbySlot.occupied && slot.peerId == peer && !CharacterTakenByOther(slot.lobbySlot.id, characterId)) {
            slot.lobbySlot.characterId =
                slot.lobbySlot.characterId == characterId ? Character::CharacterId::None : characterId;
            return true;
        }
    }
    return false;
}

bool ServerLobby::CharacterTakenByOther(uint32_t playerId, const Character::CharacterId characterId) {
    for (const auto &slot : m_slots) {
        if (slot.lobbySlot.id != playerId && slot.lobbySlot.characterId == characterId) {
            return true;
        }
    }
    return false;
}

void ServerLobby::SetReady(network::PeerId peer, bool ready) {
    for (auto &slot : m_slots) {
        if (slot.lobbySlot.occupied && slot.peerId == peer) {
            slot.lobbySlot.ready = ready;
            return;
        }
    }
}

bool ServerLobby::AllReady() const {
    for (const auto &slot : m_slots) {
        if (slot.lobbySlot.occupied && !slot.lobbySlot.ready)
            return false;
    }
    return true;
}

int ServerLobby::PlayerCount() const {
    int count = 0;
    for (const auto &slot : m_slots)
        if (slot.lobbySlot.occupied)
            count++;
    return count;
}

const LobbySlot *ServerLobby::GetSlot(network::PeerId peer) const {
    for (const auto &slot : m_slots) {
        if (slot.lobbySlot.occupied && slot.peerId == peer)
            return &slot;
    }
    return nullptr;
}
