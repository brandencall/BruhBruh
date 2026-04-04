#include "game_client.hpp"

void GameClient::Initialize() {
    InitWindow(1280, 720, "BruhBruh");
    SetTextureFilter(GetFontDefault().texture, TEXTURE_FILTER_POINT);
    SetWindowState(FLAG_VSYNC_HINT);
    if (GetFPS() == 0) {
        int monitorHz = GetMonitorRefreshRate(GetCurrentMonitor());
        SetTargetFPS(monitorHz > 0 ? monitorHz : 60);
    }

    m_gameScene.OnEnter();
}

void GameClient::Update() {
    m_running = !WindowShouldClose();
    if (!m_running)
        return;

    float dt = GetFrameTime();

    network::InboundPacket pkt;
    while (m_transport.recv(pkt))
        m_handler.Dispatch(pkt.data, pkt.size);

    m_gameScene.Update(dt);
    m_gameScene.Render();
}

void GameClient::Start(const char *ip, int port) {
    Connect(ip, port);
    m_running = true;
    while (m_running) {
        Update();
    }
}

void GameClient::Disconnect() {
    network::DisconnectPacket packet{};
    packet.header.type = network::PacketType::Disconnect;
    packet.playerId = m_gameScene.GetCurrentPlayerId(); // expose a getter
    m_transport.send(network::PEER_SERVER, &packet, sizeof(packet));
}

GameClient::~GameClient() {
    Disconnect();
    CloseWindow();
    // GameScene destructor handles its own Unload()
}

void GameClient::Connect(const char *ip, int port) { m_transport.connect(ip, port); }
