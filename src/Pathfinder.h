#pragma once

#include <functional>
#include <queue>
#include <utility>

#include "World.h"

typedef std::function<float(World* world, std::deque<Position>, std::vector<Position>)> PathfinderHeuristicFn;

/// @brief Only have one pathfinder per world, a check is performed with a warning.
class PathFinder {
  protected:
    World* world;

    void Setup();

    PathfinderHeuristicFn Heuristic;

    Position current_position;
    float current_cost;
    float current_heuristic;
    int current_goal_path = 0;

    std::unordered_map<PositionHashable, int> checked_count;

    typedef std::tuple<float, int, Position> HeapTuple;
    std::priority_queue<HeapTuple, std::vector<HeapTuple>, std::greater<HeapTuple>> open_set;

    std::vector<std::unordered_map<PositionHashable, Position>> previous;
    std::vector<std::unordered_map<PositionHashable, float>> lowest_cost;
    std::vector<std::vector<Position>> progress;
    std::vector<int> goal_progress;
    std::vector<std::vector<Position>> goal_paths;

  public:
    PathFinder(World* world, PathfinderHeuristicFn heuristic_fn);
    ~PathFinder();

    /// @brief READ ONLY
    World* get_world();

    Position get_current_position();
    int get_goal_progress();
    float get_current_cost();
    float get_current_heuristic();
    /// @return The current path with the shortest distance found, IN REVERSE ORDER
    std::deque<Position> get_current_path();
    std::vector<Position> get_current_goal_path();

    bool completed();
    bool failed();

    int checks(Position pos);

    void Step();
};

namespace Dijkstra {
float Heuristic(World* world, std::deque<Position> incomplete_path, std::vector<Position> goal_path);
}

namespace AStar {
float Heuristic(World* world, std::deque<Position> incomplete_path, std::vector<Position> goal_path);
}

namespace DijkstraCrow {
float Heuristic(World* world, std::deque<Position> incomplete_path, std::vector<Position> goal_path);
}

namespace DijkstraFolly {
float Heuristic(World* world, std::deque<Position> incomplete_path, std::vector<Position> goal_path);
}
