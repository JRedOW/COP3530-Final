#include "raylib.h"
#include "World.h"
#include "Pathfinder.h"
#include<iostream>

using namespace std;

const int TILE_SIZE = 32;

const Color GRASS_COLOR = GREEN;
const Color PATH_COLOR = DARKGRAY;
const Color BRIDGE_COLOR = BEIGE;
const Color RIVER_COLOR = BLUE;
const Color TREE_COLOR = BROWN;
const Color SPAWN_COLOR = RED;
const Color GOAL_COLOR = GOLD;
const Color DESTINATION_COLOR = PURPLE;
const Color CURRENT_PATH_COLOR = Fade(RED, 0.5f);
const Color OPTIMAL_PATH_COLOR = Fade(BLACK, 0.5f);

void ResetWorld(World* &world, PathFinder &pathfinder, int selectedAlgorithm) {
    delete world; // Free the current World object
    world = new World({30, 15}, {0, 0}, {19, 14}); // Create a new World object

    // Reapply weights and goals
    world->set_weight({10, 10}, 10);
    world->set_weight({5, 5}, 2000);
    world->set_weight({15, 10}, 1.5);
    world->set_weight({14, 7}, 1.5);
    world->set_weight({13, 6}, 10);
    world->set_weight({12, 8}, 2.0);

    world->add_goal({15, 4});
    world->add_goal({1, 6});

    // Reinitialize PathFinder
    pathfinder = PathFinder(world, selectedAlgorithm == 0 ? AStar::Heuristic : Dijkstra::Heuristic);
}

int main() {
    // Initialize Raylib
    InitWindow(800, 600, "The Legend of Alberta");
    SetTargetFPS(60);

    // Create World and PathFinder objects
    World* world = new World({30, 15}, {0, 0}, {19, 14});
    world->set_weight({10, 10}, 10);   // Example: river
    world->set_weight({5, 5}, 2000);  // Example: tree
    world->set_weight({15, 10}, 1.5); // Bridge
    world->set_weight({14, 7}, 1.5);
    world->set_weight({13, 6}, 10);
    world->set_weight({14, 4}, 2.0);

    world->add_goal({15, 4});         // Add goals
    world->add_goal({1, 6});

    PathFinder pathfinder(world, AStar::Heuristic); // Default to A*

    // Dropdown and GUI state variables
    const char *algorithms[] = { "A*", "Dijkstra" };
    bool selectedAlgorithm = false; // 0 = A*, 1 = Dijkstra
    bool dropdownActive = true;
    Rectangle dropdownRect = {400, 575, 150, 30};
    Rectangle restartButton = {600, 560, 150, 30};
    Rectangle startButton = {600, 500, 150, 30}; // Add restart button

    std::deque<Position> optimalPath;
    bool showOptimalPath = false;

    while (!WindowShouldClose()) {
        Vector2 mousePos = GetMousePosition();

        // Handle dropdown interaction
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            if (CheckCollisionPointRec(mousePos, dropdownRect)) {
                        selectedAlgorithm = !selectedAlgorithm; // Choose algorithm
                        cout << selectedAlgorithm << endl;
                        pathfinder = PathFinder(world, selectedAlgorithm == true ? AStar::Heuristic : Dijkstra::Heuristic);
                        showOptimalPath = false; // Reset optimal path visualization
            }
        }

        // Handle start button
        if (CheckCollisionPointRec(mousePos, startButton) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            // Find optimal path
            while (!pathfinder.completed() && !pathfinder.failed()) {
                pathfinder.Step();
            }
            optimalPath = pathfinder.get_current_path();
            showOptimalPath = true; // Enable optimal path visualization
        }

        // Handle restart button
        if (CheckCollisionPointRec(mousePos, restartButton) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            ResetWorld(world, pathfinder, selectedAlgorithm);
            optimalPath.clear();    // Clear any existing path
            showOptimalPath = false; // Reset visualization
        }

        // Rendering
        BeginDrawing();
        ClearBackground(BLACK);

        // Draw tiles
        for (int y = 0; y < world->get_size().second; ++y) {
            for (int x = 0; x < world->get_size().first; ++x) {
                Position pos = {x, y};
                float weight = world->get_weight(pos);
                Color tileColor;

                // Select tile color based on weight
                if (weight == 1.0f) tileColor = PATH_COLOR;
                else if (weight == 1.5f) tileColor = BRIDGE_COLOR;
                else if (weight == 2.0f) tileColor = GRASS_COLOR;
                else if (weight == 10.0f) tileColor = RIVER_COLOR;
                else if (weight >= 1001.0f) tileColor = TREE_COLOR;
                else tileColor = GRASS_COLOR;  // Default

                DrawRectangle(x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE, tileColor);
            }
        }

        // Draw spawn, goals, and destination
        DrawRectangle(world->get_spawn().first * TILE_SIZE, world->get_spawn().second * TILE_SIZE, TILE_SIZE, TILE_SIZE, SPAWN_COLOR);
        for (auto goal : world->get_goals()) {
            DrawRectangle(goal.first * TILE_SIZE, goal.second * TILE_SIZE, TILE_SIZE, TILE_SIZE, GOAL_COLOR);
        }
        DrawRectangle(world->get_destination().first * TILE_SIZE, world->get_destination().second * TILE_SIZE, TILE_SIZE, TILE_SIZE, DESTINATION_COLOR);

        // Draw optimal path
        if (showOptimalPath) {
            for (auto pos : optimalPath) {
                DrawRectangle(pos.first * TILE_SIZE, pos.second * TILE_SIZE, TILE_SIZE, TILE_SIZE, OPTIMAL_PATH_COLOR);
            }
        }

        DrawRectangleRec(dropdownRect, LIGHTGRAY);
        DrawText("Toggle Algorithm", dropdownRect.x + 16, dropdownRect.y + 4, 16, BLACK);


        // Draw start button
        DrawRectangleRec(startButton, GREEN);
        DrawText("Start", startButton.x + 50, startButton.y + 5, 20, WHITE);

        // Draw restart button
        DrawRectangleRec(restartButton, RED);
        DrawText("Restart", restartButton.x + 35, restartButton.y + 5, 20, WHITE);

        // Draw informational text
        DrawText(TextFormat("Toggle to change Algorithm: %s", algorithms[selectedAlgorithm]), 10, 500, 20, WHITE);
        DrawText("Click 'Start' to find the optimal path.", 10, 525, 20, WHITE);
        DrawText("Click 'Restart' to clear the board!", 10, 550, 20, WHITE);


        EndDrawing();
    }

    delete world; // Free allocated memory
    CloseWindow();
    return 0;
}
