#include "game.hpp"

int main(void) {
    Game game;
    while (game.GameRunning()) {
        game.Update();
    }
}
