#pragma once

#include <unordered_map>
#include <utility>
#include <vector>

typedef std::pair<int, int> Position;
typedef int PositionHashable;

int distance(Position a, Position b);

class World {
  private:
    float default_weight = 1;
    std::pair<int, int> size;

    Position spawn;
    Position destination;
    std::vector<Position> goals;

    std::unordered_map<PositionHashable, float> weighted_map;

  public:
    World(std::pair<int, int> size, Position spawn, Position destination);

    ~World();

    World(const char* filename);
    void save_world(const char* filename);

    /// @brief DO NOT MODIFY UNLESS YOU KNOW WHAT YOUR DOING
    bool locked = false;

    void set_default_weight(float weight);

    std::pair<int, int> get_size();

    Position get_spawn();
    void set_spawn(Position spawn);

    Position get_destination();
    void set_destination(Position destination);

    /// @brief DO NOT MODIFY, GOD WHY CAN'T YOU NOT HAVE MODIFICATION SAFETY LIKE RUST
    std::vector<Position> get_goals();
    void add_goal(Position goal);
    void remove_goal(Position goal);

    void set_weight(Position pos, float weight);
    float get_weight(Position pos);

    PositionHashable get_position_hashable(Position pos);
    Position get_position(PositionHashable hash);
};

// World* BIG_OL_WORLD();
