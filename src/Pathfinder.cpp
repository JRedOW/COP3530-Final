#include <algorithm>
#include <iostream>
#include <tuple>
#include <unordered_set>

#include "Pathfinder.h"

PathFinder::PathFinder(World* world, HeuristicFnDef heuristic_fn) {
    if (world == nullptr) {
        std::cout << "PathFinder was given a nullptr world" << std::endl;

        return;
    }

    if (world->locked) {
        std::cout << "PathFinder was given a locked world" << std::endl;

        return;
    }

    this->world = world;
    this->Heuristic = heuristic_fn;

    world->locked = true;

    Setup();
}

PathFinder::~PathFinder() {
    world->locked = false;
}

void PathFinder::Setup() {
    current_position = world->get_spawn();

    std::vector<Position> goal_path = world->get_goals();

    if (world->get_goals().size() == 0) {
        goal_paths.push_back({current_position, world->get_destination()});
    } else {
        std::vector<Position> goal_path_final = {current_position};
        goal_path_final.insert(goal_path_final.end(), goal_path.begin(), goal_path.end());
        goal_path_final.push_back(world->get_destination());
        goal_paths.push_back(goal_path_final);
    }

    if (world->get_goals().size() > 0) {
        // Build Goal Paths Using QuickPerm (quickperm.org)
        std::vector<unsigned int> p(world->get_goals().size() + 1);
        unsigned int i, j;
        Position tmp;
        for (i = 0; i < world->get_goals().size(); i++) {
            p[i] = i;
        }
        p[world->get_goals().size()] = world->get_goals().size();
        i = 1;
        while (i < world->get_goals().size()) {
            p[i]--;
            j = i % 2 * p[i];
            std::swap(goal_path[j], goal_path[i]);

            std::vector<Position> goal_path_final = {current_position};
            goal_path_final.insert(goal_path_final.end(), goal_path.begin(), goal_path.end());
            goal_path_final.push_back(world->get_destination());
            goal_paths.push_back(goal_path_final);

            i = 1;
            while (!p[i]) {
                p[i] = i;
                i++;
            }
        }
    }

    for (int i = 0; i < goal_paths.size(); i++) {
        previous.push_back({});
        lowest_cost.push_back({});
        progress.push_back({});
        goal_progress.push_back(0);

        lowest_cost[i][world->get_position_hashable(current_position)] = 0;

        current_cost = 0;
        current_heuristic = Heuristic(world, get_current_path(), goal_paths[i]);

        open_set.push({current_heuristic, i, current_position});
    }
}

World* PathFinder::get_world() {
    return world;
}

std::pair<int, int> PathFinder::get_current_position() {
    return current_position;
}

float PathFinder::get_current_cost() {
    return current_cost;
}

float PathFinder::get_current_heuristic() {
    return current_heuristic;
}

std::deque<std::pair<int, int>> PathFinder::get_current_path() {
    std::deque<std::pair<int, int>> current_path = {current_position};

    while (previous[current_goal_path].find(world->get_position_hashable(current_path.back())) !=
           previous[current_goal_path].end()) {
        current_path.push_back(previous[current_goal_path][world->get_position_hashable(current_path.back())]);
    }

    if (progress[current_goal_path].size() > 0)
        current_path.insert(current_path.end(), ++progress[current_goal_path].begin(),
                            progress[current_goal_path].end());

    return current_path;
}

std::vector<Position> PathFinder::get_current_goal_path() {
    return goal_paths[current_goal_path];
}

bool PathFinder::completed() {
    if (current_position != world->get_destination())
        return false;

    auto path = get_current_path();

    int i = 0;
    for (auto pos = path.rbegin(); pos != path.rend(); pos++) {
        if (i < goal_paths[current_goal_path].size() && *pos == goal_paths[current_goal_path][i])
            i++;
    }

    if (i < goal_paths[current_goal_path].size())
        return false;

    return true;
}

bool PathFinder::failed() {
    return open_set.empty();
}

int PathFinder::checks(Position pos) {
    return checked_count[world->get_position_hashable(pos)];
}

void PathFinder::Step() {
    auto weight = std::get<0>(open_set.top());
    auto goal_path = std::get<1>(open_set.top());
    auto position = std::get<2>(open_set.top());

    current_position = position;
    current_goal_path = goal_path;
    current_cost = lowest_cost[goal_path][world->get_position_hashable(current_position)];
    current_heuristic = Heuristic(world, get_current_path(), goal_paths[goal_path]);
    checked_count[world->get_position_hashable(current_position)]++;

    if (completed() || failed())
        return;

    open_set.pop();

    if (current_position == goal_paths[goal_path][goal_progress[goal_path]]) {
        auto current_path = get_current_path();

        std::priority_queue<HeapTuple, std::vector<HeapTuple>, std::greater<HeapTuple>> new_open_set = {};
        while (!open_set.empty()) {
            if (std::get<1>(open_set.top()) != goal_path)
                new_open_set.push(open_set.top());

            open_set.pop();
        }

        open_set = new_open_set;

        lowest_cost[goal_path].clear();
        previous[goal_path].clear();
        lowest_cost[goal_path][world->get_position_hashable(current_position)] = current_cost;

        progress[goal_path] = std::vector<Position>(current_path.begin(), current_path.end());
        goal_progress[goal_path]++;
    }

    std::vector<Position> neighbors = {
        {current_position.first + 1, current_position.second},
        {current_position.first - 1, current_position.second},
        {current_position.first, current_position.second + 1},
        {current_position.first, current_position.second - 1},
    };

    for (auto neighbor : neighbors) {
        PositionHashable neighbor_hashable = world->get_position_hashable(neighbor);

        if (neighbor.first < 0 || neighbor.first >= world->get_size().first || neighbor.second < 0 ||
            neighbor.second >= world->get_size().second)
            continue;

        if (world->get_weight(neighbor) >= 1000)
            continue;

        auto new_path = get_current_path();
        new_path.insert(new_path.begin(), neighbor);
        float new_weight = current_cost + world->get_weight(neighbor);
        float new_heuristic = Heuristic(world, new_path, goal_paths[goal_path]);

        // This finds the optimal path WITH NO REPETITION, will need to work some more (or maybe add switch) the
        // find the optimal path with "loops" and a check to stop overflows
        if (lowest_cost[goal_path].find(neighbor_hashable) == lowest_cost[goal_path].end() ||
            new_weight < lowest_cost[goal_path][neighbor_hashable]) {
            lowest_cost[goal_path][neighbor_hashable] = new_weight;
            previous[goal_path][neighbor_hashable] = current_position;

            open_set.push({new_weight + new_heuristic, goal_path, neighbor});
        }
    }
}

float Dijkstra::Heuristic(World* world, std::deque<Position> incomplete_path, std::vector<Position> goal_path) {
    return 0;
}

float AStar::Heuristic(World* world, std::deque<Position> incomplete_path, std::vector<Position> goal_path) {
    int i = 0;
    for (auto pos = incomplete_path.rbegin(); pos != incomplete_path.rend(); pos++) {
        if (i < goal_path.size() && goal_path[i] == *pos)
            i++;
    }

    Position current_position = incomplete_path.front();

    if (i == goal_path.size())
        return 0;

    float distance_needed = 0;

    while (i < goal_path.size()) {
        distance_needed += distance(current_position, goal_path[i]);
        current_position = goal_path[i];

        i++;
    }

    return distance_needed;
}
