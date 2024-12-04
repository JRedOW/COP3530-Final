#include "Pathfinder.h"
#include "World.h"
#include "raylib.h"
#include <iostream>

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

const int LINE_SEPERATION = 3;
const Color PATHFINDER_COLOR = ORANGE;

std::tuple<World*, PathFinder*> ResetWorld(World* world, PathFinder* pathfinder, int selectedAlgorithm,
                                           PathfinderHeuristicFn algorithmFns[]) {
    World* new_world = new World({30, 15}, {0, 0}, {19, 14}); // Create a new World object

    // Reapply weights and goals
    new_world->set_weight({10, 10}, 10);
    new_world->set_weight({5, 5}, 2000);
    new_world->set_weight({15, 10}, 1.5);
    new_world->set_weight({14, 7}, 1.5);
    new_world->set_weight({13, 6}, 10);
    new_world->set_weight({12, 8}, 2.0);

    new_world->add_goal({15, 4});
    new_world->add_goal({1, 6});

    // Reinitialize PathFinder
    PathFinder* new_pathfinder = new PathFinder(new_world, algorithmFns[selectedAlgorithm]);

    // Free old objects
    if (pathfinder != nullptr)
        delete pathfinder;
    if (world != nullptr)
        delete world;

    return std::make_tuple(new_world, new_pathfinder);
}

int main() {
    // Initialize Raylib
    InitWindow(800, 600, "The Legend of Alberta");
    SetTargetFPS(60);

    // Create World and PathFinder objects
    World* world = new World({30, 15}, {0, 0}, {19, 14});
    world->set_weight({10, 10}, 10);  // Example: river
    world->set_weight({5, 5}, 2000);  // Example: tree
    world->set_weight({15, 10}, 1.5); // Bridge
    world->set_weight({14, 7}, 1.5);
    world->set_weight({13, 6}, 10);
    world->set_weight({14, 4}, 2.0);

    world->add_goal({15, 4}); // Add goals
    world->add_goal({1, 6});

    PathFinder* pathfinder = new PathFinder(world, AStar::Heuristic); // Default to A*

    // Dropdown and GUI state variables
    const char* algorithms[] = {"A*", "Dijkstra", "Dijkstra's Crow", "Dijkstra's Folly"};
    PathfinderHeuristicFn algorithmFns[] = {AStar::Heuristic, Dijkstra::Heuristic, DijkstraCrow::Heuristic,
                                            DijkstraFolly::Heuristic};
    auto algorithmCount = sizeof(algorithms) / sizeof(*algorithms);
    int selectedAlgorithm = 0; // 0 = A*, 1 = Dijkstra, 2 = Dijkstra's Crow, 3 = Dijkstra's Folly
    bool dropdownActive = true;
    Rectangle dropdownRect = {400, 575, 150, 30};
    Rectangle restartButton = {600, 560, 150, 30};
    Rectangle startButton = {600, 500, 150, 30};

    bool runningPathfinder = false;
    int stallFrames = 1;
    int currentFrame = 0;

    while (!WindowShouldClose()) {
        // Run Pathfinder Step
        if (runningPathfinder) {
            if (!pathfinder->completed() && !pathfinder->failed()) {
                if (currentFrame % (stallFrames + 1) == 0) {
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
                cout << selectedAlgorithm << endl;

                delete pathfinder;
                pathfinder = new PathFinder(world, algorithmFns[selectedAlgorithm]);
            }

            // Handle start button
            if (CheckCollisionPointRec(mousePos, startButton)) {
                // Find optimal path
                runningPathfinder = !runningPathfinder;
            }

            // Handle restart button
            if (CheckCollisionPointRec(mousePos, restartButton)) {
                runningPathfinder = false;

                std::tie(world, pathfinder) = ResetWorld(world, pathfinder, selectedAlgorithm, algorithmFns);
            }
        }

        /****    RENDERING    ****/

        BeginDrawing();
        ClearBackground(BLACK);

        // Draw tiles
        for (int y = 0; y < world->get_size().second; ++y) {
            for (int x = 0; x < world->get_size().first; ++x) {
                Position pos = {x, y};
                float weight = world->get_weight(pos);
                Color tileColor;

                // Select tile color based on weight
                if (weight == 1.0f)
                    tileColor = PATH_COLOR;
                else if (weight == 1.5f)
                    tileColor = BRIDGE_COLOR;
                else if (weight == 2.0f)
                    tileColor = GRASS_COLOR;
                else if (weight == 10.0f)
                    tileColor = RIVER_COLOR;
                else if (weight >= 1001.0f)
                    tileColor = TREE_COLOR;
                else
                    tileColor = GRASS_COLOR; // Default

                DrawRectangle(x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE, tileColor);

                float checks = (float)pathfinder->checks(pos);
                if (checks > 0)
                    DrawRectangle(x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE, Fade(RED, checks / (checks + 5)));
            }
        }

        // Draw spawn, goals, and destination
        DrawRectangle(world->get_spawn().first * TILE_SIZE, world->get_spawn().second * TILE_SIZE, TILE_SIZE, TILE_SIZE,
                      SPAWN_COLOR);
        for (auto goal : world->get_goals()) {
            DrawRectangle(goal.first * TILE_SIZE, goal.second * TILE_SIZE, TILE_SIZE, TILE_SIZE, GOAL_COLOR);
        }
        DrawRectangle(world->get_destination().first * TILE_SIZE, world->get_destination().second * TILE_SIZE,
                      TILE_SIZE, TILE_SIZE, DESTINATION_COLOR);

        // Draw optimal path
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
                DrawLine(
                    path[path_i + 1].first * TILE_SIZE + TILE_SIZE / 2 + ((loopbacks / 2) * LINE_SEPERATION * side),
                    path[path_i + 1].second * TILE_SIZE + TILE_SIZE / 2 + ((loopbacks / 2) * LINE_SEPERATION * side),
                    last_turn.first * TILE_SIZE + TILE_SIZE / 2 + ((loopbacks / 2) * LINE_SEPERATION * side),
                    last_turn.second * TILE_SIZE + TILE_SIZE / 2 + ((loopbacks / 2) * LINE_SEPERATION * side),
                    PATHFINDER_COLOR);
                last_turn = path[path_i + 1];
                traveled = 0;
            }

            if (traveled > distance(last_turn, path[path_i])) {
                int side = loopbacks % 2 == 0 ? 1 : -1;
                DrawLine(
                    path[path_i + 1].first * TILE_SIZE + TILE_SIZE / 2 + ((loopbacks / 2) * LINE_SEPERATION * side),
                    path[path_i + 1].second * TILE_SIZE + TILE_SIZE / 2 + ((loopbacks / 2) * LINE_SEPERATION * side),
                    last_turn.first * TILE_SIZE + TILE_SIZE / 2 + ((loopbacks / 2) * LINE_SEPERATION * side),
                    last_turn.second * TILE_SIZE + TILE_SIZE / 2 + ((loopbacks / 2) * LINE_SEPERATION * side),
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
        DrawLine(path[path_i].first * TILE_SIZE + TILE_SIZE / 2 + ((loopbacks / 2) * LINE_SEPERATION * side),
                 path[path_i].second * TILE_SIZE + TILE_SIZE / 2 + ((loopbacks / 2) * LINE_SEPERATION * side),
                 last_turn.first * TILE_SIZE + TILE_SIZE / 2 + ((loopbacks / 2) * LINE_SEPERATION * side),
                 last_turn.second * TILE_SIZE + TILE_SIZE / 2 + ((loopbacks / 2) * LINE_SEPERATION * side),
                 PATHFINDER_COLOR);

        size_t goal_path_i = 0;
        auto goal_path = pathfinder->get_current_goal_path();
        while (goal_path_i < goal_path.size() - 1) {
            Color color = goal_path_i + 1 < pathfinder->get_goal_progress() ? BLUE : RED;
            if (pathfinder->completed())
                color = GREEN;

            DrawLine(goal_path[goal_path_i].first * TILE_SIZE + TILE_SIZE / 2,
                     goal_path[goal_path_i].second * TILE_SIZE + TILE_SIZE / 2,
                     goal_path[goal_path_i + 1].first * TILE_SIZE + TILE_SIZE / 2,
                     goal_path[goal_path_i + 1].second * TILE_SIZE + TILE_SIZE / 2, color);

            goal_path_i++;
        }

        DrawRectangleRec(dropdownRect, LIGHTGRAY);
        DrawText("Toggle Algorithm", (int)dropdownRect.x + 16, (int)dropdownRect.y + 4, 16, BLACK);

        // Draw start button
        if (runningPathfinder) {
            DrawRectangleRec(startButton, RED);
            DrawText("Stop", (int)startButton.x + 50, (int)startButton.y + 5, 20, WHITE);
        } else {
            DrawRectangleRec(startButton, GREEN);
            DrawText("Start", (int)startButton.x + 50, (int)startButton.y + 5, 20, WHITE);
        }

        // Draw restart button
        DrawRectangleRec(restartButton, RED);
        DrawText("Restart", (int)restartButton.x + 35, (int)restartButton.y + 5, 20, WHITE);

        // Draw informational text
        DrawText(TextFormat("Toggle to change Algorithm: %s", algorithms[selectedAlgorithm]), 10, 500, 20, WHITE);
        DrawText("Click 'Start' to find the optimal path.", 10, 525, 20, WHITE);
        DrawText("Click 'Restart' to clear the board!", 10, 550, 20, WHITE);

        EndDrawing();

        currentFrame++;
    }

    delete world; // Free allocated memory
    CloseWindow();
    return 0;
}
