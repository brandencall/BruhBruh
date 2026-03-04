#include "client/game_client.hpp"
#include "game_server.hpp"
#include <iostream>

void RunServer() {
    std::cout << "Running the server" << std::endl;
    GameServer gameServer;
    gameServer.Start(54000);
    gameServer.RunServer();
}

void RunClient() {
    GameClient gameClient;
    gameClient.Connect("127.0.0.1", 54000);
    gameClient.SendJoin();

    while (gameClient.GameRunning()) {
        gameClient.Update();
    }
}

int main(int argc, char **argv) {
    bool isServer = false;
    bool isClient = false;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "--server")
            isServer = true;
        else if (arg == "--client")
            isClient = true;
    }

    if (isServer)
        RunServer();
    else if (isClient)
        RunClient();
    else
        RunClient(); // default
}
