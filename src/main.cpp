#include "game.hpp"

// Should go inside renderer
// typedef struct Sprite {
//    Texture2D texture;
//    Rectangle destinationRec;
//} Sprite;

int main(void) {
    // InitWindow(800, 450, "raylib [core] example - basic window");
    // SetTargetFPS(60);

    // Texture2D test_player_texture = LoadTexture("assets/test_player.png");
    // Sprite player = (Sprite){.texture = test_player_texture,
    //                          .destinationRec = (Rectangle){.x = 10.0, .y = 100.0, .width = 32.0, .height = 32.0}};

    // while (!WindowShouldClose()) {
    //     BeginDrawing();
    //     ClearBackground(RAYWHITE);
    //     DrawTexturePro(player.texture, {0, 0, 16, 16}, player.destinationRec, (Vector2){0.0f, 0.0f}, 0.0f, WHITE);
    //     EndDrawing();
    // }

    // UnloadTexture(test_player_texture);
    // CloseWindow();

    Game game;
    while (game.GameRunning()) {
        game.Update();
    }
}
