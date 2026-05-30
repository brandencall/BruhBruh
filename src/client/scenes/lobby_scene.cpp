#include "lobby_scene.hpp"
#include "../../network/packets/lobby_packets.hpp"
#include "../ui/screens/friends_invite_screen.hpp"
#include "../ui/screens/pause_menu_screen.hpp"
#include "../utils/text_utils.hpp"
#include "raylib.h"
#include <cstdint>
#include <cstring>
#include <iostream>
#include <memory>
#include <unordered_map>

LobbyScene::LobbyScene(Game &game) : m_game(game) {}

void LobbyScene::OnEnter() {
    using PT = network::PacketType;
    NetworkMessageHandler *handler = m_game.GetNetworkMessageHandler();
    handler->Register(PT::JoinResponse, [this](const char *b) { HandleJoinResponse(b); });
    handler->Register(PT::PlayerJoined, [this](const char *b) { HandlePlayerJoined(b); });
    handler->Register(PT::PlayerReady, [this](const char *b) { HandlePlayerReady(b); });
    handler->Register(PT::CharacterSelected, [this](const char *b) { HandleCharacterSelected(b); });
    handler->Register(PT::LobbyState, [this](const char *b) { HandleLobbyState(b); });
    handler->Register(PT::StartGame, [this](const char *b) { HandleGameStarting(b); });
    handler->Register(PT::GameBegin, [this](const char *b) { HandleGameBegin(b); });
    // Host Disconnect needs to be handled in both lobby and game scenes.
    // TODO: Might not have to register it and Unregister it every scene
    handler->Register(PT::HostDisconnected, [this](const char *b) { HandleHostDisconnected(b); });

    // TODO: Replace these with the actual character icons
    m_icons[Character::CharacterId::Tonts] = LoadTexture("assets/characters/tmp/Tonts.png");
    m_icons[Character::CharacterId::Raff] = LoadTexture("assets/characters/tmp/Chavz.png");
    m_icons[Character::CharacterId::Hodge] = LoadTexture("assets/characters/tmp/Hodge.png");
    m_icons[Character::CharacterId::JJ] = LoadTexture("assets/characters/tmp/Big_J.png");

    m_pendingJoin = true;

    m_game.GetAudioSystem()->InitLobby(onGameStarting);
}

void LobbyScene::OnExit() {
    Scene::OnExit();

    NetworkMessageHandler *handler = m_game.GetNetworkMessageHandler();
    using PT = network::PacketType;
    handler->Unregister(PT::JoinResponse);
    handler->Unregister(PT::PlayerJoined);
    handler->Unregister(PT::PlayerReady);
    handler->Unregister(PT::CharacterSelected);
    handler->Unregister(PT::LobbyState);
    handler->Unregister(PT::StartGame);
    handler->Unregister(PT::GameBegin);
    handler->Unregister(PT::HostDisconnected);

    m_ui.Clear();
    m_game.GetAudioSystem()->UnloadLobby();

    for (auto &[id, tex] : m_icons)
        UnloadTexture(tex);
}

void LobbyScene::Update(float dt) {
    if (m_pendingJoin) {
        m_pendingJoin = false;
        if (!m_game.GetSessionManager()->GetLobby())
            SendLocalJoin();
        else
            SendJoin();
        return;
    }

    if (!m_joined && !m_game.GetSessionManager()->GetLobby()) {
        SendLocalJoin();
        return;
    }

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

    if (m_game.GetSessionManager()->GetLobby() && m_game.GetSessionManager()->GetLobby()->IsLocalPlayerHost())
        UpdateInviteButton(mousePos);

    if (IsKeyPressed(KEY_ESCAPE)) {
        UI::PauseMenuConfig cfg;
        cfg.context = UI::MenuContext::Lobby;
        cfg.onLeave = [this]() { SendDisconnect(); };
        cfg.onQuitDesktop = [this]() { SendDisconnect(); };
        m_ui.Push(std::make_unique<UI::PauseMenu>(m_game, m_ui, cfg));
    }
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
        m_ui.Push(std::make_unique<UI::FriendsInviteScreen>(*m_game.GetSessionManager()->GetLobby()));
}

// Corner-bracket helper (same as pause menu)
static void DrawLobbyCorners(Rectangle r, Color c, float len = 8.f, float thick = 2.f) {
    int t = (int)thick, L = (int)len;
    int x0 = (int)r.x - 1, y0 = (int)r.y - 1;
    int x1 = (int)(r.x + r.width), y1 = (int)(r.y + r.height);
    DrawRectangle(x0, y0, L, t, c);
    DrawRectangle(x0, y0, t, L, c);
    DrawRectangle(x1 - L, y0, L, t, c);
    DrawRectangle(x1 - t, y0, t, L, c);
    DrawRectangle(x0, y1 - t, L, t, c);
    DrawRectangle(x0, y1 - L, t, L, c);
    DrawRectangle(x1 - L, y1 - t, L, t, c);
    DrawRectangle(x1 - t, y1 - L, t, L, c);
}

// ─────────────────────────────────────────────────────────────────────────────
// Render
// ─────────────────────────────────────────────────────────────────────────────

void LobbyScene::Render() {
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();

    BeginDrawing();
    ClearBackground(kLobbyBg);

    // ── Vignette (mirrors MainMenuScene::RenderBackground) ───────────────────
    for (int r = screenH; r > 0; r -= screenH / 8) {
        unsigned char alpha = (unsigned char)(80.f * (1.f - (float)r / screenH));
        DrawCircle(screenW / 2, screenH / 2, (float)r, {0, 0, 0, alpha});
    }

    // ── Title ────────────────────────────────────────────────────────────────
    int titleSz = (int)(screenH * 0.072f);
    utils::DrawTextCentered("LOBBY", screenW * 0.5f, screenH * 0.06f, titleSz, kLobbyTextPrimary);

    // Thin divider under title (mirrors main menu)
    int lineW = screenW * 0.18f;
    int lineY = (int)(screenH * 0.155f);
    DrawLine(screenW / 2 - lineW / 2, lineY, screenW / 2 + lineW / 2, lineY, kLobbyDivider);

    // ── Player slots ─────────────────────────────────────────────────────────
    std::unordered_map<Character::CharacterId, uint32_t> takenCharacters;
    for (int i = 0; i < MAX_PLAYERS; ++i) {
        int x = (int)(screenW * 0.125f + (i * screenW * 0.203f));
        int y = (int)(screenH * 0.2f);
        RenderPlayerSlot(i, m_players[i], x, y, screenW, screenH);
        if (m_players[i].id != -1 && m_players[i].characterId != Character::CharacterId::None)
            takenCharacters[m_players[i].characterId] = m_players[i].id;
    }

    // ── Character selection section ──────────────────────────────────────────
    int subSz = (int)(screenH * 0.026f);
    utils::DrawTextCentered("SELECT CHARACTER", screenW * 0.5f, screenH * 0.655f, subSz, kLobbyTextMuted);
    int iconDivW = (int)(screenW * 0.30f);
    int iconDivY = (int)(screenH * 0.648f);
    DrawLine(screenW / 2 - iconDivW / 2, iconDivY, screenW / 2 + iconDivW / 2, iconDivY, kLobbyDivider);

    Vector2 mousePos = GetMousePosition();
    RenderCharacterIcons(takenCharacters, mousePos);

    // ── Ready hint ────────────────────────────────────────────────────────────
    const char *hint = m_ready ? "READY  -  press SPACE to cancel" : "Press SPACE when ready";
    Color hintC = m_ready ? kLobbyReady : kLobbyTextMuted;
    utils::DrawTextCentered(hint, screenW * 0.5f, screenH * 0.925f, subSz, hintC);

    // ── Countdown overlay ─────────────────────────────────────────────────────
    if (m_gameStarting) {
        DrawRectangle(0, 0, screenW, screenH, {0, 0, 0, 160});
        int fontSize = (int)(screenH * 0.177f);
        const char *countdownText = TextFormat("%d", (int)std::ceil(m_countdownTimer));
        utils::DrawTextCentered(countdownText, screenW * 0.5f, (screenH - fontSize) * 0.5f, fontSize,
                                kLobbyTextPrimary);
        int msgSz = (int)(screenH * 0.028f);
        utils::DrawTextCentered("Game starting...", screenW * 0.5f, screenH * 0.62f, msgSz, kLobbyTextMuted);
    }

    // ── Invite button (host only) ─────────────────────────────────────────────
    if (m_game.GetSessionManager()->GetLobby() && m_game.GetSessionManager()->GetLobby()->IsLocalPlayerHost())
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
        int iconSize = (int)(screenW * 0.0625f);
        int padding = (int)(screenW * 0.015f);
        int totalWidth = (int)m_icons.size() * (iconSize + padding) - padding;
        int startX = (screenW - totalWidth) / 2;
        int x = startX + iconPos * (iconSize + padding);
        int y = (int)(screenH * 0.708f);

        Rectangle iconRect = {(float)x, (float)y, (float)iconSize, (float)iconSize};
        auto it = takenCharacters.find(icon.first);
        bool takenByOther = (it != takenCharacters.end() && it->second != m_localPlayerId);
        bool hovered = CheckCollisionPointRec(mousePos, iconRect) && !m_ready;

        DrawRectangleRec(iconRect, kLobbySlotFill);

        if (takenByOther) {
            DrawTexturePro(icon.second, {0, 0, (float)icon.second.width, (float)icon.second.height}, iconRect, {0, 0},
                           0.f, {80, 80, 90, 180});
            DrawRectangleLinesEx(iconRect, 1.5f, {50, 50, 70, 255});
        } else {
            if (hovered)
                DrawRectangleRec(iconRect, ColorAlpha(kLobbyAccentHover, 0.15f));
            DrawTexturePro(icon.second, {0, 0, (float)icon.second.width, (float)icon.second.height}, iconRect, {0, 0},
                           0.f, WHITE);
            DrawRectangleLinesEx(iconRect, 1.5f, hovered ? kLobbyAccentHover : kLobbyBorder);
            if (hovered)
                DrawLobbyCorners(iconRect, kLobbyAccentHover, 6.f, 2.f);
        }
        ++iconPos;
    }
}

void LobbyScene::RenderInviteButton(int screenW, int screenH, Vector2 mousePos) {
    float btnW = screenW * 0.18f;
    float btnH = screenH * 0.072f;
    Rectangle btnRect = {screenW * 0.5f - btnW * 0.5f, screenH * 0.82f + 20, btnW, btnH};

    bool hovered = CheckCollisionPointRec(mousePos, btnRect) && !m_ui.HasScreenOfType<UI::FriendsInviteScreen>();
    Color fill = hovered ? kLobbyAccentHover : kLobbyAccent;
    Color border = hovered ? Color{120, 160, 255, 255} : kLobbyBorder;

    DrawRectangleRec(btnRect, fill);
    DrawRectangleLinesEx(btnRect, 1.5f, border);

    int labelSz = (int)(screenH * 0.026f);
    utils::DrawTextCentered("Invite Friends", btnRect.x + btnRect.width * 0.5f,
                            btnRect.y + btnRect.height * 0.5f - labelSz * 0.5f, labelSz, WHITE);
}

void LobbyScene::RenderPlayerSlot(int slot, const state::LobbySlotState &player, int x, int y, int screenW,
                                  int screenH) {
    int slotW = (int)(screenW * 0.171f);
    int slotH = (int)(screenH * 0.416f);

    // Slot background
    DrawRectangle(x, y, slotW, slotH, kLobbySlotFill);

    // Local player gets a blue accent border; empty slots are very muted
    bool isLocal = (player.id == m_localPlayerId);
    Color borderCol = (player.id == -1) ? Color{40, 40, 60, 255} : isLocal ? kLobbyAccentHover : kLobbyBorder;
    DrawRectangleLinesEx({(float)x, (float)y, (float)slotW, (float)slotH}, 1.5f, borderCol);

    if (isLocal)
        DrawLobbyCorners({(float)x, (float)y, (float)slotW, (float)slotH}, kLobbyAccentHover, 10.f, 2.f);

    if (player.id == -1) {
        int waitSz = (int)(screenH * 0.020f);
        utils::DrawTextCentered("Waiting...", x + slotW * 0.5f, y + slotH * 0.433f, waitSz, {60, 60, 80, 255});
        return;
    }

    // Player name + rule
    int nameSz = (int)(screenH * 0.022f);
    DrawText(player.name, x + (int)(slotW * 0.06f), y + (int)(slotH * 0.06f), nameSz, kLobbyTextPrimary);
    DrawLine(x + (int)(slotW * 0.06f), y + (int)(slotH * 0.13f), x + (int)(slotW * 0.94f), y + (int)(slotH * 0.13f),
             kLobbyDivider);

    RenderSelectedCharacter(player, slotW, slotH, x, y);

    // Ready badge
    int badgeSz = (int)(screenH * 0.022f);
    int badgeX = x + (int)(slotW * 0.1f);
    int badgeY = y + (int)(slotH * 0.82f);
    int badgeW = (int)(slotW * 0.8f);
    int badgeH = (int)(slotH * 0.10f);
    if (player.ready) {
        DrawRectangle(badgeX, badgeY, badgeW, badgeH, {30, 80, 40, 200});
        DrawRectangleLinesEx({(float)badgeX, (float)badgeY, (float)badgeW, (float)badgeH}, 1.f, kLobbyReady);
        utils::DrawTextCentered("READY", x + slotW * 0.5f, badgeY + badgeH * 0.5f - badgeSz * 0.5f, badgeSz,
                                kLobbyReady);
    } else {
        DrawRectangle(badgeX, badgeY, badgeW, badgeH, {80, 25, 25, 200});
        DrawRectangleLinesEx({(float)badgeX, (float)badgeY, (float)badgeW, (float)badgeH}, 1.f, kLobbyNotReady);
        utils::DrawTextCentered("NOT READY", x + slotW * 0.5f, badgeY + badgeH * 0.5f - badgeSz * 0.5f, badgeSz,
                                kLobbyNotReady);
    }
}

void LobbyScene::RenderSelectedCharacter(const state::LobbySlotState &player, int slotW, int slotH, int x, int y) {
    auto it = m_icons.find(player.characterId);
    if (it != m_icons.end() && it->first != Character::CharacterId::None) {
        int iconSize = (int)(slotW * 0.6f);
        int iconX = x + (slotW - iconSize) / 2;
        int iconY = y + (int)(slotH * 0.2f);
        Rectangle iconRect = {(float)iconX, (float)iconY, (float)iconSize, (float)iconSize};
        DrawTexturePro(it->second, {0, 0, (float)it->second.width, (float)it->second.height}, iconRect, {0, 0}, 0.f,
                       WHITE);
        // DrawRectangleLinesEx(iconRect, 1.f, kLobbyBorder);
    } else {
        int noCharSz = (int)(slotH * 0.030f);
        utils::DrawTextCentered("No character", x + slotW * 0.5f, y + slotH * 0.44f, noCharSz, {60, 60, 80, 255});
    }
}

void LobbyScene::SendJoin() {
    network::JoinLobbyPacket pkt{};
    pkt.header.type = network::PacketType::JoinLobby;
    const char *name = SteamFriends()->GetPersonaName();
    strncpy(pkt.name, name, MAX_PLAYER_NAME_LEN - 1);
    m_game.GetTransport()->send(network::PEER_SERVER, &pkt, sizeof(pkt));
}

void LobbyScene::SendLocalJoin() {
    network::JoinLobbyPacket pkt{};
    pkt.header.type = network::PacketType::JoinLobby;
    const char *name = "Player";
    strncpy(pkt.name, name, MAX_PLAYER_NAME_LEN - 1);
    m_game.GetTransport()->send(network::PEER_SERVER, &pkt, sizeof(pkt));
}

void LobbyScene::SendDisconnect() {
    network::DisconnectPacket packet{};
    packet.header.type = network::PacketType::Disconnect;
    m_game.GetTransport()->send(network::PEER_SERVER, &packet, sizeof(packet));
}

void LobbyScene::FlipReadyState() {
    network::PlayerReadyPacket pkt{};
    pkt.header.type = network::PacketType::PlayerReady;
    pkt.playerId = m_localPlayerId;
    pkt.playerReady = !m_ready;
    m_game.GetTransport()->send(network::PEER_SERVER, &pkt, sizeof(pkt));
}

void LobbyScene::OnCharacterSelected(const Character::CharacterId &character) {
    network::CharacterSelectedPacket pkt{};
    pkt.header.type = network::PacketType::CharacterSelected;
    pkt.playerId = m_localPlayerId;
    pkt.characterId = character;
    m_game.GetTransport()->send(network::PEER_SERVER, &pkt, sizeof(pkt));
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
    m_previousCountdown = m_countdownTimer;
    m_countdownTimer = (int)std::ceil(pkt->countdown);
    m_maxCountdown = std::max(m_maxCountdown, m_countdownTimer);
    m_gameStarting = true;
    onGameStarting.Publish({m_previousCountdown, m_countdownTimer, m_maxCountdown});
}

void LobbyScene::HandleGameBegin(const char *buf) {
    m_game.GetSessionManager()->CreateGame(m_players[m_localPlayerId]);
}

void LobbyScene::HandleHostDisconnected(const char *buf) { m_game.GetSessionManager()->ReturnToMainMenu(); }
