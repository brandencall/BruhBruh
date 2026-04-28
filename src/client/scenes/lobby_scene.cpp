#include "lobby_scene.hpp"
#include "../../network/packets/lobby_packets.hpp"
#include "../ui/screens/friends_invite_screen.hpp"
#include "../utils/text_utils.hpp"
#include "raylib.h"
#include <cstdint>
#include <cstring>
#include <memory>
#include <unordered_map>

LobbyScene::LobbyScene(Client::EventHub &events, network::ITransport &transport, NetworkMessageHandler &handler,
                       SessionManager &sessionManager)
    : m_events(events), m_transport(transport), m_handler(handler), m_sessionManager(sessionManager) {}

void LobbyScene::OnEnter() {
    using PT = network::PacketType;
    m_handler.Register(PT::JoinResponse, [this](const char *b) { HandleJoinResponse(b); });
    m_handler.Register(PT::PlayerJoined, [this](const char *b) { HandlePlayerJoined(b); });
    m_handler.Register(PT::PlayerReady, [this](const char *b) { HandlePlayerReady(b); });
    m_handler.Register(PT::CharacterSelected, [this](const char *b) { HandleCharacterSelected(b); });
    m_handler.Register(PT::LobbyState, [this](const char *b) { HandleLobbyState(b); });
    m_handler.Register(PT::StartGame, [this](const char *b) { HandleGameStarting(b); });
    m_handler.Register(PT::GameBegin, [this](const char *b) { HandleGameBegin(b); });

    // TODO: Replace these with the actual character icons
    m_icons[Character::CharacterId::Tonts] = LoadTexture("assets/characters/tmp/Tonts.png");
    m_icons[Character::CharacterId::Raff] = LoadTexture("assets/characters/tmp/Chavz.png");
    m_icons[Character::CharacterId::Hodge] = LoadTexture("assets/characters/tmp/Hodge.png");
    m_icons[Character::CharacterId::JJ] = LoadTexture("assets/characters/tmp/Big_J.png");

    if (!m_sessionManager.GetLobby().IsLocalPlayerHost())
        SendJoin();
}

void LobbyScene::OnExit() {
    Scene::OnExit();

    using PT = network::PacketType;
    m_handler.Unregister(PT::JoinResponse);
    m_handler.Unregister(PT::PlayerJoined);
    m_handler.Unregister(PT::PlayerReady);
    m_handler.Unregister(PT::CharacterSelected);
    m_handler.Unregister(PT::LobbyState);
    m_handler.Unregister(PT::StartGame);
    m_handler.Unregister(PT::GameBegin);

    m_ui.Clear();

    for (auto &[id, tex] : m_icons)
        UnloadTexture(tex);
}

void LobbyScene::Update(float dt) {
    m_ui.Update(dt);

    if (m_ui.BlocksGameInput())
        return;

    // Build taken-characters map once so both selection checks can use it
    std::unordered_map<Character::CharacterId, uint32_t> takenCharacters;
    for (int i = 0; i < MAX_PLAYERS; ++i) {
        if (m_players[i].id != -1 && m_players[i].characterId != Character::CharacterId::None)
            takenCharacters[m_players[i].characterId] = m_players[i].id;
    }

    Vector2 mousePos = GetMousePosition();

    if (IsKeyPressed(KEY_SPACE) && !m_gameStarting)
        FlipReadyState();

    UpdateCharacterSelection(mousePos, takenCharacters);

    if (m_sessionManager.GetLobby().IsLocalPlayerHost())
        UpdateInviteButton(mousePos);
}

void LobbyScene::UpdateCharacterSelection(Vector2 mousePos,
                                          const std::unordered_map<Character::CharacterId, uint32_t> &takenCharacters) {

    if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || m_ready)
        return;

    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();

    int iconPos = 0;
    for (const auto &icon : m_icons) {
        int iconSize = screenW * 0.0625f;
        int padding = screenW * 0.015f;
        int totalWidth = (int)m_icons.size() * (iconSize + padding) - padding;
        int startX = (screenW - totalWidth) / 2;
        int x = startX + iconPos * (iconSize + padding);
        int y = screenH * 0.708f;

        Rectangle iconRect = {(float)x, (float)y, (float)iconSize, (float)iconSize};

        // Only fire if this icon isn't taken by someone else
        auto it = takenCharacters.find(icon.first);
        bool takenByOther = (it != takenCharacters.end() && it->second != m_localPlayerId);

        if (!takenByOther && CheckCollisionPointRec(mousePos, iconRect))
            OnCharacterSelected(icon.first);

        ++iconPos;
    }
}

void LobbyScene::UpdateInviteButton(Vector2 mousePos) {
    if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        return;

    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();
    int btnW = screenW * 0.18f;
    int btnH = screenH * 0.06f;

    Rectangle btnRect = {screenW * 0.5f - btnW * 0.5f, screenH * 0.82f, (float)btnW, (float)btnH};

    if (CheckCollisionPointRec(mousePos, btnRect))
        m_ui.Push(std::make_unique<UI::FriendsInviteScreen>(m_sessionManager.GetLobby()));
}

void LobbyScene::Render() {
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();

    BeginDrawing();
    ClearBackground(BLACK);

    utils::DrawTextCentered("LOBBY", screenW * 0.5f, 60, 32, WHITE);
    utils::DrawTextCentered("Press SPACE when ready", screenW * 0.5f, screenH * 0.916f, 20, GRAY);

    std::unordered_map<Character::CharacterId, uint32_t> takenCharacters;
    for (int i = 0; i < MAX_PLAYERS; ++i) {
        int x = screenW * 0.125f + (i * screenW * 0.203f);
        int y = screenH * 0.2f;
        RenderPlayerSlot(i, m_players[i], x, y, screenW, screenH);
        if (m_players[i].id != -1 && m_players[i].characterId != Character::CharacterId::None)
            takenCharacters[m_players[i].characterId] = m_players[i].id;
    }

    utils::DrawTextCentered("Select Character:", screenW * 0.5f, screenH * 0.667f, 20, WHITE);

    Vector2 mousePos = GetMousePosition(); // read-only, for hover highlights only
    RenderCharacterIcons(takenCharacters, mousePos);

    if (m_gameStarting) {
        const char *countdownText = TextFormat("%d", (int)std::ceil(m_countdownTimer));
        int fontSize = screenH * 0.177f;
        utils::DrawTextCentered(countdownText, screenW * 0.5f, (screenH - fontSize) * 0.5f, fontSize, YELLOW);
    }

    if (m_sessionManager.GetLobby().IsLocalPlayerHost())
        RenderInviteButton(screenW, screenH, mousePos);

    m_ui.Render();

    EndDrawing();
}

void LobbyScene::RenderCharacterIcons(const std::unordered_map<Character::CharacterId, uint32_t> &takenCharacters,
                                      Vector2 mousePos) {

    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();

    int iconPos = 0;
    for (const auto &icon : m_icons) {
        int iconSize = screenW * 0.0625f;
        int padding = screenW * 0.015f;
        int totalWidth = (int)m_icons.size() * (iconSize + padding) - padding;
        int startX = (screenW - totalWidth) / 2;
        int x = startX + iconPos * (iconSize + padding);
        int y = screenH * 0.708f;

        Rectangle iconRect = {(float)x, (float)y, (float)iconSize, (float)iconSize};
        auto it = takenCharacters.find(icon.first);
        bool takenByOther = (it != takenCharacters.end() && it->second != m_localPlayerId);
        bool hovered = CheckCollisionPointRec(mousePos, iconRect);

        if (takenByOther) {
            DrawTexturePro(icon.second, {0, 0, (float)icon.second.width, (float)icon.second.height}, iconRect, {0, 0},
                           0.0f, DARKGRAY);
            DrawRectangleLinesEx(iconRect, 2, DARKGRAY);
        } else {
            if (hovered)
                DrawRectangleRec(iconRect, ColorAlpha(WHITE, 0.2f));

            DrawTexturePro(icon.second, {0, 0, (float)icon.second.width, (float)icon.second.height}, iconRect, {0, 0},
                           0.0f, WHITE);
            DrawRectangleLinesEx(iconRect, 2, hovered ? YELLOW : GRAY);
        }
        ++iconPos;
    }
}

void LobbyScene::RenderInviteButton(int screenW, int screenH, Vector2 mousePos) {
    int btnW = screenW * 0.18f;
    int btnH = screenH * 0.06f;

    Rectangle btnRect = {screenW * 0.5f - btnW * 0.5f, screenH * 0.82f + 20, (float)btnW, (float)btnH};
    bool hovered = CheckCollisionPointRec(mousePos, btnRect);

    DrawRectangleRec(btnRect, hovered ? ColorAlpha(WHITE, 0.2f) : ColorAlpha(GRAY, 0.2f));
    DrawRectangleLinesEx(btnRect, 2, hovered ? YELLOW : GRAY);
    utils::DrawTextCentered("Invite Friends", btnRect.x + btnRect.width * 0.5f, btnRect.y + btnRect.height * 0.5f, 20,
                            WHITE);
}

void LobbyScene::SendJoin() {
    network::JoinLobbyPacket pkt{};
    pkt.header.type = network::PacketType::JoinLobby;
    const char *name = SteamFriends()->GetPersonaName();
    strncpy(pkt.name, name, MAX_PLAYER_NAME_LEN - 1);
    m_transport.send(network::PEER_SERVER, &pkt, sizeof(pkt));
}

void LobbyScene::FlipReadyState() {
    network::PlayerReadyPacket pkt{};
    pkt.header.type = network::PacketType::PlayerReady;
    pkt.playerId = m_localPlayerId;
    pkt.playerReady = !m_ready;
    m_transport.send(network::PEER_SERVER, &pkt, sizeof(pkt));
}

void LobbyScene::OnCharacterSelected(const Character::CharacterId &character) {
    network::CharacterSelectedPacket pkt{};
    pkt.header.type = network::PacketType::CharacterSelected;
    pkt.playerId = m_localPlayerId;
    pkt.characterId = character;
    m_transport.send(network::PEER_SERVER, &pkt, sizeof(pkt));
}

void LobbyScene::HandleJoinResponse(const char *buf) {
    auto *pkt = reinterpret_cast<const network::JoinResponsePacket *>(buf);
    m_localPlayerId = pkt->playerId;
    m_players[m_localPlayerId].id = pkt->playerId;
    m_players[m_localPlayerId].characterId = pkt->characterId;
    m_joined = true;
    strcpy(m_players[m_localPlayerId].name, pkt->name);
}

void LobbyScene::HandlePlayerJoined(const char *buf) {
    auto *pkt = reinterpret_cast<const network::PlayerJoinedPacket *>(buf);
    m_players[pkt->playerId].id = pkt->playerId;
    m_players[pkt->playerId].characterId = pkt->characterId;
    strcpy(m_players[pkt->playerId].name, pkt->name);
}

void LobbyScene::HandlePlayerReady(const char *buf) {
    auto *pkt = reinterpret_cast<const network::PlayerReadyPacket *>(buf);
    if (pkt->playerId == m_localPlayerId)
        m_ready = pkt->playerReady;
}

void LobbyScene::HandleCharacterSelected(const char *buf) {
    auto *pkt = reinterpret_cast<const network::CharacterSelectedPacket *>(buf);
    m_players[pkt->playerId].characterId = pkt->characterId;
}

void LobbyScene::HandleLobbyState(const char *buf) {
    auto *pkt = reinterpret_cast<const network::LobbyStatePacket *>(buf);
    for (uint8_t i = 0; i < MAX_PLAYERS; ++i) {
        m_players[i].id = pkt->lobby[i].id;
        m_players[i].ready = pkt->lobby[i].ready;
        m_players[i].characterId = pkt->lobby[i].characterId;
        strcpy(m_players[i].name, pkt->lobby[i].name);
    }
}

void LobbyScene::HandleGameStarting(const char *buf) {
    auto *pkt = reinterpret_cast<const network::StartGamePacket *>(buf);
    m_countdownTimer = pkt->countdown;
    m_gameStarting = true;
}

void LobbyScene::HandleGameBegin(const char *buf) { m_sessionManager.CreateGame(m_players[m_localPlayerId]); }

void LobbyScene::RenderPlayerSlot(int slot, const state::LobbySlotState &player, int x, int y, int screenW,
                                  int screenH) {
    int slotW = screenW * 0.171f;
    int slotH = screenH * 0.416f;

    Color borderColor = player.id != -1 ? WHITE : DARKGRAY;
    DrawRectangleLines(x, y, slotW, slotH, borderColor);

    if (player.id == -1) {
        utils::DrawTextCentered("Waiting...", x + slotW * 0.5f, y + slotH * 0.433f, 16, DARKGRAY);
        return;
    }

    DrawText(player.name, x + slotW * 0.045f, y + slotH * 0.066f, 18, WHITE);
    RenderSelectedCharacter(player, slotW, slotH, x, y);

    if (player.ready)
        utils::DrawTextCentered("READY", x + slotW * 0.5f, y + slotH * 0.8f, 20, GREEN);
    else
        utils::DrawTextCentered("NOT READY", x + slotW * 0.5f, y + slotH * 0.8f, 20, RED);

    if (player.id == m_localPlayerId)
        DrawRectangleLines(x - 2, y - 2, slotW + 4, slotH + 4, YELLOW);
}

void LobbyScene::RenderSelectedCharacter(const state::LobbySlotState &player, int slotW, int slotH, int x, int y) {
    auto it = m_icons.find(player.characterId);
    if (it != m_icons.end() && it->first != Character::CharacterId::None) {
        int iconSize = slotW * 0.6f;
        int iconX = x + (slotW - iconSize) * 0.5f;
        int iconY = y + slotH * 0.2f;
        Rectangle iconRect = {(float)iconX, (float)iconY, (float)iconSize, (float)iconSize};
        DrawTexturePro(it->second, {0, 0, (float)it->second.width, (float)it->second.height}, iconRect, {0, 0}, 0.0f,
                       WHITE);
    } else {
        utils::DrawTextCentered("No character", x + slotW * 0.5f, y + slotH * 0.4f, 14, DARKGRAY);
    }
}
