#pragma once

#include <chrono>
#include <functional>
#include <iostream>
#include <set>
#include <vector>

using namespace std::chrono;

namespace engine
{

enum direction
{
    NORTH, EAST, SOUTH, WEST, STATIC
};

struct coordinate
{
    coordinate() = default;
    coordinate(int x_, int y_) : x(x_), y(y_) {}
    ~coordinate() = default;
    coordinate(const coordinate&) = default;
    coordinate(coordinate&&) = default;
    coordinate& operator=(const coordinate&) = default;
    coordinate& operator=(coordinate&&) = default;

    bool operator==(const coordinate& other) const
    {
        if (other.x == x && other.y == y)
            return true;
    
        return false;
    }

    int x { 0 }; 
    int y { 0 };
    auto& operator+=(coordinate other)
    {
        x += other.x;
        y += other.y;
        return *this;
    }
};

struct bounding_box
{
    coordinate top_left {0, 0};
    coordinate dimensions {1, 1};
};

struct extents
{
    coordinate minima { 0, 0 };
    coordinate maxima { 0, 0 };
};

struct world_object
{
    world_object() = delete;
    world_object(coordinate position) : position_(position) {}
    world_object(coordinate position, std::function<void(void)> logic) : position_(position), logic_(logic) {}
    world_object(char symbol, coordinate position, std::function<void(void)> logic) : symbol_(symbol), position_(position), logic_(logic) {} // TODO: deprecate
    virtual ~world_object() = default;
    world_object(const world_object&) = default;
    world_object(world_object&&) = default;
    world_object& operator=(const world_object& other) = default;
    world_object& operator=(world_object&& other) = default;

    virtual void update(system_clock::duration delta_t) { logic_(); };
    virtual extents get_extents() const = 0;

    char symbol_ { '?' }; // TODO: deprecate
    coordinate position_;
    std::function<void(void)> logic_; // TODO: void(duration) ? 
    // TODO: callbacks?

    // TODO: use for object management
    static std::size_t instance_counter;
};

std::size_t world_object::instance_counter = 0;

struct i_movable
{
    virtual void set_move_intent(direction dir) = 0;
    virtual direction get_move_intent(bool reset=false) = 0;
    virtual bool move() = 0;
    virtual coordinate get_position() = 0; 
    virtual bool allow_oob() const = 0;
};

struct render_info
{
    bounding_box bb;
    std::vector<char> model; // copy
    char transparency { '&' };
    std::size_t z_order { 1 };
    bool is_background { false };
};

struct i_drawable
{
    virtual render_info get_render_info() = 0;
};

struct i_collider
{
    virtual bounding_box get_bounding_box() const = 0;
    virtual void on_collision(i_collider* other) = 0;
};

using hardpoints = std::vector<coordinate>;

// struct attack info: location, hardpoints, homing, dumbfire, direction, ...
struct attack_info
{
    direction dir;
    coordinate position;
};

struct i_attacker
{
    virtual bool attack() = 0;
    virtual attack_info get_attack_info() const = 0;
    virtual const hardpoints& get_hardpoints() const = 0;
};

} // engine::
