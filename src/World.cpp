#include <iostream>
#include <utility>

#include "World.h"

int distance(Position a, Position b) {
    return abs(a.first - b.first) + abs(a.second - b.second);
}

World::World(std::pair<int, int> size, Position spawn, Position destination) : size(size) {
    this->spawn = spawn;
    this->destination = destination;
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
