#include "raylib.h"

#include "Pathfinder.h"
#include "World.h"

#define SCREEN_WIDTH (800)
#define SCREEN_HEIGHT (450)

#define WINDOW_TITLE "The Legend of Alberta"

#ifndef ASSETS_PATH
#define ASSETS_PATH "./assets"
#endif

int main(void) {
    World world = World({10, 10}, {5, 0}, {5, 9});
    world.add_goal({0, 0});
    world.add_goal({0, 2});
    world.add_goal({0, 9});
    PathFinder pathfinder = PathFinder(&world, AStar::Heuristic);

    while (!pathfinder.completed() && !pathfinder.failed()) {
        pathfinder.Step();
    }

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, WINDOW_TITLE);

    SetTargetFPS(60);

    Texture2D texture = LoadTexture(ASSETS_PATH "test.png");

    while (!WindowShouldClose()) {
        BeginDrawing();

        ClearBackground(RAYWHITE);

        const int texture_x = SCREEN_WIDTH / 2 - texture.width / 2;
        const int texture_y = SCREEN_HEIGHT / 2 - texture.height / 2;
        DrawTexture(texture, texture_x, texture_y, WHITE);

        const char* text = "OMG! IT WORKS!";
        const Vector2 text_size = MeasureTextEx(GetFontDefault(), text, 20, 1);
        DrawText(text, SCREEN_WIDTH / 2 - text_size.x / 2, texture_y + texture.height + text_size.y + 10, 20, BLACK);

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
