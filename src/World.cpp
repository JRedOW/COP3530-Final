#include <algorithm>
#include <fstream>
#include <iostream>
#include <utility>

#include "World.h"

int distance(Position a, Position b) {
    return abs(a.first - b.first) + abs(a.second - b.second);
}

World::World(std::pair<int, int> size, Position spawn, Position destination) {
    this->size = size;
    this->spawn = spawn;
    this->destination = destination;
}

World::~World() {
    if (locked)
        std::cerr << "World is locked, baaaddddd **** may go down" << std::endl;
}

World::World(const char* filename) {
    std::ifstream file(filename, std::ios::binary);

    if (!file.is_open()) {
        std::cerr << "Error opening file!" << std::endl;

        size = {1, 2};
        spawn = {0, 0};
        destination = {0, 1};

        return;
    }

    int width, height;
    file.read(reinterpret_cast<char*>(&width), sizeof(int));
    file.read(reinterpret_cast<char*>(&height), sizeof(int));
    size = {width, height};

    int spawn_x, spawn_y;
    file.read(reinterpret_cast<char*>(&spawn_x), sizeof(int));
    file.read(reinterpret_cast<char*>(&spawn_y), sizeof(int));
    spawn = {spawn_x, spawn_y};

    int destination_x, destination_y;
    file.read(reinterpret_cast<char*>(&destination_x), sizeof(int));
    file.read(reinterpret_cast<char*>(&destination_y), sizeof(int));
    destination = {destination_x, destination_y};

    int goals_count;
    file.read(reinterpret_cast<char*>(&goals_count), sizeof(int));
    for (int i = 0; i < goals_count; i++) {
        int goal_x, goal_y;
        file.read(reinterpret_cast<char*>(&goal_x), sizeof(int));
        file.read(reinterpret_cast<char*>(&goal_y), sizeof(int));
        goals.push_back({goal_x, goal_y});
    }

    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            float weight;
            file.read(reinterpret_cast<char*>(&weight), sizeof(float));
            weighted_map[get_position_hashable({x, y})] = weight;
        }
    }

    file.close();
}

void World::save_world(const char* filename) {
    std::ofstream file(filename, std::ios::binary);

    if (!file.is_open()) {
        std::cerr << "Error opening file!" << std::endl;

        return;
    }

    int width = size.first;
    int height = size.second;
    file.write(reinterpret_cast<char*>(&width), sizeof(int));
    file.write(reinterpret_cast<char*>(&height), sizeof(int));

    int spawn_x = spawn.first;
    int spawn_y = spawn.second;
    file.write(reinterpret_cast<char*>(&spawn_x), sizeof(int));
    file.write(reinterpret_cast<char*>(&spawn_y), sizeof(int));

    int destination_x = destination.first;
    int destination_y = destination.second;
    file.write(reinterpret_cast<char*>(&destination_x), sizeof(int));
    file.write(reinterpret_cast<char*>(&destination_y), sizeof(int));

    int goals_count = goals.size();
    file.write(reinterpret_cast<char*>(&goals_count), sizeof(int));
    for (auto goal : goals) {
        int goal_x = goal.first;
        int goal_y = goal.second;
        file.write(reinterpret_cast<char*>(&goal_x), sizeof(int));
        file.write(reinterpret_cast<char*>(&goal_y), sizeof(int));
    }

    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            float weight = get_weight({x, y});
            file.write(reinterpret_cast<char*>(&weight), sizeof(float));
        }
    }

    file.close();
}

std::pair<int, int> World::get_size() {
    return size;
}

Position World::get_spawn() {
    return spawn;
}

void World::set_spawn(Position spawn) {
    if (locked) {
        std::cerr << "World is locked, can not set spawn" << std::endl;

        return;
    }

    this->spawn = spawn;
}

Position World::get_destination() {
    return destination;
}

void World::set_destination(Position destination) {
    if (locked) {
        std::cerr << "World is locked, can not set destination" << std::endl;

        return;
    }

    this->destination = destination;
}

std::vector<Position> World::get_goals() {
    return goals;
}

void World::add_goal(Position goal) {
    if (locked) {
        std::cerr << "World is locked, can not add goal" << std::endl;

        return;
    }

    goals.push_back(goal);
}

void World::remove_goal(Position goal) {
    if (locked) {
        std::cerr << "World is locked, can not remove goal" << std::endl;

        return;
    }

    goals.erase(std::remove(goals.begin(), goals.end(), goal), goals.end());
}

void World::set_default_weight(float weight) {
    if (locked) {
        std::cerr << "World is locked, can not set default weight" << std::endl;

        return;
    }

    default_weight = weight;
}

void World::set_weight(Position pos, float weight) {
    if (locked) {
        std::cerr << "World is locked, can not set weight" << std::endl;

        return;
    }

    weighted_map[get_position_hashable(pos)] = weight;
}

float World::get_weight(Position pos) {
    PositionHashable pos_hash = get_position_hashable(pos);

    if (weighted_map.find(pos_hash) != weighted_map.end()) {
        return weighted_map[pos_hash];
    }

    return default_weight;
}

PositionHashable World::get_position_hashable(Position pos) {
    return pos.first * size.second + pos.second;
}

Position World::get_position(PositionHashable hash) {
    return {hash / size.second, hash % size.second};
}

/*
World* BIG_OL_WORLD() {
    // Get Data From Here: https://keesiemeijer.github.io/maze-generator/#generate
    // With This In Console:
    //
    // let out = "";
    // for (let i = 1; i < mazeNodes.matrix.length; i++) {
    //     out += "\""
    //     out += mazeNodes.matrix[i].slice(1);
    //     out += "\","
    // }

    char* data[] = {""};

    std::pair<int, int> size = {400, 250};

    World* world = new World(size, {0, 0}, {size.first - 1, size.second - 2});

    for (int x = 0; x < size.first; x++) {
        for (int y = 0; y < size.second; y++) {
            char c = data[x][y];
            if (c == '0') {
                world->set_weight({x, y}, 1.0f);
            } else {
                world->set_weight({x, y}, 1221.0f);
            }
        }
    }

    return world;
}
*/
