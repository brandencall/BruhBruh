#include "client/game.hpp"
#include "client/game_client.hpp"
#include "network/network_message_handler.hpp"
#include "steam/steam_api.h"
#include <iostream>

void RunServer() {
    std::cout << "Running the server" << std::endl;
    GameServer gameServer;
    gameServer.Start(54000);
    gameServer.RunServer();
}

void RunClient() {
    NetworkMessageHandler handler;
    GameClient gameClient(handler);
    gameClient.Start("127.0.0.1", 54000);
    Game game;
    game.RunLocal(gameClient);
}

int main(int argc, char **argv) {

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "--server") {
            RunServer();
            return 0;
        } else if (arg == "--client") {
            RunClient();
            return 0;
        }
    }

    if (!SteamAPI_Init()) {
        std::cerr << "SteamAPI_Init failed. Is Steam running?\n";
        return 1;
    }

    Game game;
    game.Run();

    SteamAPI_Shutdown();
}

// #include "client/game_client.hpp"
// #include "game_server.hpp"
// #include <iostream>
//
// void RunServer() {
//     std::cout << "Running the server" << std::endl;
//     GameServer gameServer;
//     gameServer.Start(54000);
//     gameServer.RunServer();
// }
//
// void RunClient() {
//     GameClient gameClient;
//     gameClient.Initialize();
//     gameClient.Start("127.0.0.1", 54000);
// }
//
// int main(int argc, char **argv) {
//     bool isServer = false;
//     bool isClient = false;
//
//     for (int i = 1; i < argc; ++i) {
//         std::string arg = argv[i];
//
//         if (arg == "--server")
//             isServer = true;
//         else if (arg == "--client")
//             isClient = true;
//     }
//
//     if (isServer)
//         RunServer();
//     else if (isClient)
//         RunClient();
//     else
//         RunClient(); // default
// }
