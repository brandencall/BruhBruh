#pragma once
#include "../config.hpp"
#include "../shared/network/ITransport.hpp"
#include "../shared/state/lobby_slot_state.hpp"
#include "characters/character_types.hpp"
#include <cstdint>

struct LobbySlot {
    network::PeerId peerId;
    state::LobbySlotState lobbySlot;
};

class ServerLobby {
  public:
    // Returns assigned slot index, or -1 if full
    int AddPlayer(network::PeerId peer, const char *name, uint32_t playerId);
    int AddPlayer(network::PeerId peer, uint32_t playerId);
    void RemovePlayer(network::PeerId peer);
    bool TrySetCharacter(network::PeerId peer, Character::CharacterId characterId);
    bool CharacterTaken(uint32_t playerId, const Character::CharacterId characterId);
    void SetReady(network::PeerId peer, bool ready);

    bool AllReady() const;
    int PlayerCount() const;

    const LobbySlot *GetSlot(network::PeerId peer) const;
    const LobbySlot (&Slots() const)[MAX_PLAYERS] { return m_slots; }

  private:
    LobbySlot m_slots[MAX_PLAYERS] = {};
};
