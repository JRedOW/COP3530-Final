#include "Pathfinder.h"
#include "World.h"
#include "raylib.h"
#include <iostream>

#ifndef ASSETS_PATH
#define ASSETS_PATH "./assets"
#endif

const int LINE_SEPERATION = 3;
const Color PATHFINDER_COLOR = ORANGE;
const Color PATHFINDER_MINI_COLOR = Fade(ORANGE, 0.5f);

int main() {
    // Initialize Raylib
    InitWindow(1600, 900, "The Legend of Alberta");
    SetTargetFPS(60);

    // Initialize textures
    Texture2D path16Texture = LoadTexture(ASSETS_PATH "tiles/16/sand.png");
    Texture2D path32Texture = LoadTexture(ASSETS_PATH "tiles/32/sand.png");
    Texture2D bridge16Texture = LoadTexture(ASSETS_PATH "tiles/16/bridge.png");
    Texture2D bridge32Texture = LoadTexture(ASSETS_PATH "tiles/32/bridge.png");
    Texture2D grass16Texture = LoadTexture(ASSETS_PATH "tiles/16/grass.png");
    Texture2D grass32Texture = LoadTexture(ASSETS_PATH "tiles/32/grass.png");
    Texture2D river16Texture = LoadTexture(ASSETS_PATH "tiles/16/river.png");
    Texture2D river32Texture = LoadTexture(ASSETS_PATH "tiles/32/river.png");
    Texture2D tree16Texture = LoadTexture(ASSETS_PATH "tiles/16/tree.png");
    Texture2D tree32Texture = LoadTexture(ASSETS_PATH "tiles/32/tree.png");
    Texture2D coin16Texture = LoadTexture(ASSETS_PATH "tiles/16/coin.png");
    Texture2D coin32Texture = LoadTexture(ASSETS_PATH "tiles/32/coin.png");
    Texture2D black16Texture = LoadTexture(ASSETS_PATH "tiles/16/black.png");
    Texture2D black32Texture = LoadTexture(ASSETS_PATH "tiles/32/black.png");
    Texture2D flame16Texture = LoadTexture(ASSETS_PATH "tiles/16/flame.png");
    Texture2D flame32Texture = LoadTexture(ASSETS_PATH "tiles/32/flame.png");
    Texture2D sword16Texture = LoadTexture(ASSETS_PATH "tiles/16/sword.png");
    Texture2D sword32Texture = LoadTexture(ASSETS_PATH "tiles/32/sword.png");
    Texture2D zRock16Texture = LoadTexture(ASSETS_PATH "tiles/16/Z_rock.png");
    Texture2D zRock32Texture = LoadTexture(ASSETS_PATH "tiles/32/Z_rock.png");

    // Dropdown and GUI state variables
    const char* algorithms[] = {"A*", "Dijkstra", "Dijkstra's Crow", "Dijkstra's Folly"};
    PathfinderHeuristicFn algorithmFns[] = {AStar::Heuristic, Dijkstra::Heuristic, DijkstraCrow::Heuristic,
                                            DijkstraFolly::Heuristic};
    const int algorithmCount = sizeof(algorithms) / sizeof(*algorithms);
    int selectedAlgorithm = 0; // 0 = A*, 1 = Dijkstra, 2 = Dijkstra's Crow, 3 = Dijkstra's Folly
    const char* maps[] = {"It's Dangerous To Go Alone!", "Big Boy"};
    const char* mapFiles[] = {ASSETS_PATH "worlds/its_dangerous.dat", ASSETS_PATH "worlds/big_ol_world.dat"};
    const int mapCount = sizeof(maps) / sizeof(*maps);
    int selectedMap = 0;
    const char* speeds[] = {"Slow", "Fast", "Faster", "Fastest", "Ludicrous"};
    const int stallFrames[] = {8, 5, 3, 1, 0};
    const int speedCount = sizeof(speeds) / sizeof(*speeds);
    int selectedSpeed = 0;

    bool dropdownActive = true;
    Rectangle dropdownRect = {800, 760, 150, 30};
    Rectangle mapRect = {800, 810, 150, 30};
    Rectangle speedRect = {800, 860, 150, 30};
    Rectangle startButton = {1400, 760, 150, 30};
    Rectangle stepButton = {1400, 810, 150, 30};
    Rectangle restartButton = {1400, 860, 150, 30};

    bool runningPathfinder = false;
    int currentFrame = 0;

    // Create World and PathFinder objects
    World* world = new World(mapFiles[selectedMap]);
    PathFinder* pathfinder = new PathFinder(world, algorithmFns[selectedAlgorithm]);

    while (!WindowShouldClose()) {
        // Run Pathfinder Step
        if (runningPathfinder) {
            if (!pathfinder->completed() && !pathfinder->failed()) {
                if (currentFrame % (stallFrames[selectedSpeed] + 1) == 0) {
                    pathfinder->Step();
                }
            } else {
                runningPathfinder = false;
            }
        }

        Vector2 mousePos = GetMousePosition();

        // Handle dropdown interaction
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            if (CheckCollisionPointRec(mousePos, dropdownRect)) {
                runningPathfinder = false;

                selectedAlgorithm = ++selectedAlgorithm % algorithmCount; // Choose algorithm
                std::cout << selectedAlgorithm << std::endl;

                delete pathfinder;
                pathfinder = new PathFinder(world, algorithmFns[selectedAlgorithm]);
            }

            if (CheckCollisionPointRec(mousePos, mapRect)) {
                runningPathfinder = false;

                selectedMap = ++selectedMap % mapCount; // Choose map
                std::cout << selectedMap << std::endl;

                delete pathfinder;
                delete world;
                world = new World(mapFiles[selectedMap]);
                pathfinder = new PathFinder(world, algorithmFns[selectedAlgorithm]);
            }

            if (CheckCollisionPointRec(mousePos, speedRect)) {
                selectedSpeed = ++selectedSpeed % speedCount; // Choose speed
            }

            // Handle start button
            if (CheckCollisionPointRec(mousePos, startButton)) {
                // Find optimal path
                runningPathfinder = !runningPathfinder;
            }

            // Handle step button
            if (CheckCollisionPointRec(mousePos, stepButton)) {
                // Step pathfinder
                if (!runningPathfinder) {
                    pathfinder->Step();
                } else {
                    runningPathfinder = false;

                    while (!pathfinder->completed() && !pathfinder->failed()) {
                        pathfinder->Step();
                    }
                }
            }

            // Handle restart button
            if (CheckCollisionPointRec(mousePos, restartButton)) {
                runningPathfinder = false;

                delete pathfinder;
                pathfinder = new PathFinder(world, algorithmFns[selectedAlgorithm]);
            }
        }

        // Calculate map size
        int tileSize = 32;
        int mapWidth = world->get_size().first * tileSize;
        int mapHeight = world->get_size().second * tileSize;
        if (mapWidth >= 1600 || mapHeight >= 750) {
            tileSize = 16;
            mapWidth = world->get_size().first * tileSize;
            mapHeight = world->get_size().second * tileSize;

            if (mapWidth >= 1600 || mapHeight >= 750) {
                tileSize = std::min(1600 / world->get_size().first, 750 / world->get_size().second);
                mapWidth = world->get_size().first * tileSize;
                mapHeight = world->get_size().second * tileSize;
            }
        }
        int mapOffsetX = (1600 - mapWidth) / 2;
        int mapOffsetY = (750 - mapHeight) / 2;

        /****    RENDERING    ****/

        BeginDrawing();
        ClearBackground(BLACK);

        // Draw tiles
        for (int y = 0; y < world->get_size().second; ++y) {
            for (int x = 0; x < world->get_size().first; ++x) {
                Position pos = {x, y};
                float weight = world->get_weight(pos);
                Color tileColor;
                Texture2D tileTexture;

                // Select tile color based on weight
                if (weight == 1.0f) { // Path
                    tileColor = DARKGRAY;
                    tileTexture = tileSize == 16 ? path16Texture : path32Texture;
                } else if (weight == 1.0078125f) { // Zelda Black
                    tileColor = BLACK;
                    tileTexture = tileSize == 16 ? black16Texture : black32Texture;
                } else if (weight == 1.5f) { // Bridge
                    tileColor = BEIGE;
                    tileTexture = tileSize == 16 ? bridge16Texture : bridge32Texture;
                } else if (weight == 2.0f) { // Grass
                    tileColor = GREEN;
                    tileTexture = tileSize == 16 ? grass16Texture : grass32Texture;
                } else if (weight == 10.0f) { // River
                    tileColor = BLUE;
                    tileTexture = tileSize == 16 ? river16Texture : river32Texture;
                } else if (weight == 1001.0f) { // Tree
                    tileColor = BROWN;
                    tileTexture = tileSize == 16 ? tree16Texture : tree32Texture;
                } else if (weight == 1200.0f) { // Zelda Rock
                    tileColor = BLACK;
                    tileTexture = tileSize == 16 ? zRock16Texture : zRock32Texture;
                } else if (weight == 1221.0f) { // Black Wall
                    tileColor = BLACK;
                    tileTexture = tileSize == 16 ? black16Texture : black32Texture;
                } else if (weight == 1532.0f) { // Zelda Flame
                    tileColor = BLACK;
                    tileTexture = tileSize == 16 ? flame16Texture : flame32Texture;
                } else if (weight == 2556.0f) { // Zelda Sword
                    tileColor = BLACK;
                    tileTexture = tileSize == 16 ? sword16Texture : sword32Texture;
                } else { // Default
                    tileColor = GREEN;
                    tileTexture = tileSize == 16 ? black16Texture : black32Texture;
                }

                if (tileSize >= 16) {
                    DrawTexture(tileTexture, x * tileSize + mapOffsetX, y * tileSize + mapOffsetY, WHITE);
                } else {
                    DrawRectangle(x * tileSize + mapOffsetX, y * tileSize + mapOffsetY, tileSize, tileSize, tileColor);
                }

                // Draw check count
                float checks = (float)pathfinder->checks(pos);
                if (checks > 0)
                    DrawRectangle(x * tileSize + mapOffsetX, y * tileSize + mapOffsetY, tileSize, tileSize,
                                  Fade(RED, checks / (checks + 5)));
            }
        }

        // Draw spawn, goals, and destination
        if (tileSize >= 16) {
            DrawRectangle(world->get_spawn().first * tileSize + mapOffsetX,
                          world->get_spawn().second * tileSize + mapOffsetY, tileSize, tileSize, RED);
            for (auto goal : world->get_goals())
                DrawTexture(tileSize == 16 ? coin16Texture : coin32Texture, goal.first * tileSize + mapOffsetX,
                            goal.second * tileSize + mapOffsetY, WHITE);
            DrawRectangle(world->get_destination().first * tileSize + mapOffsetX,
                          world->get_destination().second * tileSize + mapOffsetY, tileSize, tileSize, PURPLE);
        } else {
            DrawRectangle(world->get_spawn().first * tileSize + mapOffsetX,
                          world->get_spawn().second * tileSize + mapOffsetY, tileSize, tileSize, RED);
            for (auto goal : world->get_goals())
                DrawRectangle(goal.first * tileSize + mapOffsetX, goal.second * tileSize + mapOffsetY, tileSize,
                              tileSize, GOLD);
            DrawRectangle(world->get_destination().first * tileSize + mapOffsetX,
                          world->get_destination().second * tileSize + mapOffsetY, tileSize, tileSize, PURPLE);
        }

        // Draw optimal path
        if (tileSize >= 16) {
            int loopbacks = 1;
            auto path = pathfinder->get_current_path();
            size_t path_i = path.size() - 1;
            Position last_turn = Position(-1, -1);
            int traveled = 0;
            while (path_i >= 0) {
                if (last_turn.first == -1)
                    last_turn = path[path_i];

                if ((path[path_i].first != last_turn.first && path[path_i].second != last_turn.second)) {
                    int side = loopbacks % 2 == 0 ? 1 : -1;
                    DrawLine(path[path_i + 1].first * tileSize + tileSize / 2 +
                                 ((loopbacks / 2) * LINE_SEPERATION * side) + mapOffsetX,
                             path[path_i + 1].second * tileSize + tileSize / 2 +
                                 ((loopbacks / 2) * LINE_SEPERATION * side) + mapOffsetY,
                             last_turn.first * tileSize + tileSize / 2 + ((loopbacks / 2) * LINE_SEPERATION * side) +
                                 mapOffsetX,
                             last_turn.second * tileSize + tileSize / 2 + ((loopbacks / 2) * LINE_SEPERATION * side) +
                                 mapOffsetY,
                             PATHFINDER_COLOR);
                    last_turn = path[path_i + 1];
                    traveled = 0;
                }

                if (traveled > distance(last_turn, path[path_i])) {
                    int side = loopbacks % 2 == 0 ? 1 : -1;
                    DrawLine(path[path_i + 1].first * tileSize + tileSize / 2 +
                                 ((loopbacks / 2) * LINE_SEPERATION * side) + mapOffsetX,
                             path[path_i + 1].second * tileSize + tileSize / 2 +
                                 ((loopbacks / 2) * LINE_SEPERATION * side) + mapOffsetY,
                             last_turn.first * tileSize + tileSize / 2 + ((loopbacks / 2) * LINE_SEPERATION * side) +
                                 mapOffsetX,
                             last_turn.second * tileSize + tileSize / 2 + ((loopbacks / 2) * LINE_SEPERATION * side) +
                                 mapOffsetY,
                             PATHFINDER_COLOR);

                    last_turn = path[path_i + 1];

                    loopbacks++;
                }

                traveled = distance(last_turn, path[path_i]);

                if (path_i == 0)
                    break;

                path_i--;
            }
            int side = loopbacks % 2 == 0 ? 1 : -1;
            DrawLine(
                path[path_i].first * tileSize + tileSize / 2 + ((loopbacks / 2) * LINE_SEPERATION * side) + mapOffsetX,
                path[path_i].second * tileSize + tileSize / 2 + ((loopbacks / 2) * LINE_SEPERATION * side) + mapOffsetY,
                last_turn.first * tileSize + tileSize / 2 + ((loopbacks / 2) * LINE_SEPERATION * side) + mapOffsetX,
                last_turn.second * tileSize + tileSize / 2 + ((loopbacks / 2) * LINE_SEPERATION * side) + mapOffsetY,
                PATHFINDER_COLOR);
        } else {
            for (auto pos : pathfinder->get_current_path()) {
                DrawRectangle(pos.first * tileSize + mapOffsetX, pos.second * tileSize + mapOffsetY, tileSize, tileSize,
                              PATHFINDER_COLOR);
            }
        }

        size_t goal_path_i = 0;
        auto goal_path = pathfinder->get_current_goal_path();
        while (goal_path_i < goal_path.size() - 1) {
            Color color = goal_path_i + 1 < pathfinder->get_goal_progress() ? BLUE : RED;
            if (pathfinder->completed())
                color = GREEN;

            DrawLine(goal_path[goal_path_i].first * tileSize + tileSize / 2 + mapOffsetX,
                     goal_path[goal_path_i].second * tileSize + tileSize / 2 + mapOffsetY,
                     goal_path[goal_path_i + 1].first * tileSize + tileSize / 2 + mapOffsetX,
                     goal_path[goal_path_i + 1].second * tileSize + tileSize / 2 + mapOffsetY, color);

            goal_path_i++;
        }

        DrawRectangleRec(dropdownRect, LIGHTGRAY);
        DrawText("Toggle Algorithm", (int)dropdownRect.x + 16, (int)dropdownRect.y + 4, 16, BLACK);

        DrawRectangleRec(mapRect, LIGHTGRAY);
        DrawText("Toggle Map", (int)mapRect.x + 16, (int)mapRect.y + 4, 16, BLACK);

        DrawRectangleRec(speedRect, LIGHTGRAY);
        DrawText("Toggle Speed", (int)speedRect.x + 16, (int)speedRect.y + 4, 16, BLACK);

        // Draw start button
        if (runningPathfinder) {
            DrawRectangleRec(startButton, RED);
            DrawText("Stop", (int)startButton.x + 50, (int)startButton.y + 5, 20, WHITE);
        } else {
            DrawRectangleRec(startButton, GREEN);
            DrawText("Start", (int)startButton.x + 50, (int)startButton.y + 5, 20, WHITE);
        }

        // Draw step button
        if (runningPathfinder) {
            DrawRectangleRec(stepButton, ORANGE);
            DrawText("Skip", (int)stepButton.x + 50, (int)stepButton.y + 5, 20, WHITE);
        } else {
            DrawRectangleRec(stepButton, GREEN);
            DrawText("Step", (int)stepButton.x + 50, (int)stepButton.y + 5, 20, WHITE);
        }

        // Draw reset button
        DrawRectangleRec(restartButton, RED);
        DrawText("Reset", (int)restartButton.x + 35, (int)restartButton.y + 5, 20, WHITE);

        // Draw informational text
        DrawText(TextFormat("Current Algorithm: %s", algorithms[selectedAlgorithm]), 10, 750, 20, WHITE);
        DrawText(TextFormat("Current Map: %s", maps[selectedMap]), 10, 775, 20, WHITE);
        DrawText(TextFormat("Current Speed: %s", speeds[selectedSpeed]), 10, 800, 20, WHITE);
        DrawText("Click 'Start' to automatically find an optimal path.", 10, 825, 20, WHITE);
        DrawText("Click 'Step' to manually step the pathfinder.", 10, 850, 20, WHITE);
        DrawText("Click 'Reset' to clear the board!", 10, 875, 20, WHITE);

        EndDrawing();

        currentFrame++;
    }

    delete world; // Free allocated memory
    CloseWindow();
    return 0;
}
