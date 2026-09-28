#pragma once

#include <chrono>
#include <functional>
#include <vector>

using namespace std::chrono;

enum direction
{
    NORTH, EAST, SOUTH, WEST, STATIC
};

struct coordinate
{
    coordinate() = default;
    coordinate(int x_, int y_) : x(x_), y(y_) {}
    virtual ~coordinate() = default;
    coordinate(const coordinate&) = default;
    coordinate(coordinate&&) = default;
    coordinate& operator=(coordinate&) = default;
    coordinate& operator=(coordinate&&) = default;

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

coordinate get_random_coord()
{
    auto x = rand() % 79;
    auto y = rand() % 23;

    return {x, y};
}

struct world_object
{
    world_object() = delete;
    world_object(coordinate position, std::function<void(void)> logic) : position_(position), logic_(logic) {}
    world_object(char symbol, coordinate position, std::function<void(void)> logic) : symbol_(symbol), position_(position), logic_(logic) {} // TODO: deprecate
    virtual ~world_object() = default;
    world_object(const world_object&) = default;
    world_object(world_object&&) = default;
    world_object& operator=(world_object& other) = default;
    world_object& operator=(world_object&& other) = default;

    virtual void update(system_clock::duration delta_t) { logic_(); };

    char symbol_ { '?' }; // TODO: deprecate
    coordinate position_;
    std::function<void(void)> logic_; // TODO: void(duration) ? 
};

struct i_movable
{
    virtual void set_move_intent(direction dir) = 0;
    virtual direction get_move_intent(bool reset=false) = 0; // const?
    virtual bool move() = 0;
    virtual bool allow_autoscroll() = 0; // const
    virtual coordinate get_position() = 0; 
    virtual std::chrono::system_clock::duration get_speed() = 0; // const
};

struct render_info
{
    bounding_box bb;
    std::vector<char> model; // copy
    char transparency { '&' };
    std::size_t z_order { 1 };
};

struct i_drawable
{
    virtual char get_representation() = 0; // TODO: deprecate
    virtual render_info get_render_info()= 0; // const
};

struct i_collider
{
    virtual bounding_box get_bounding_box() const = 0;
    virtual void on_collision(i_collider* other) = 0;
};

bool is_collision(const i_collider& a, const i_collider& b)
{
    return false;
    
    if (&a == &b)
        return false;
        
    const auto& bb_a = a.get_bounding_box();
    const auto& bb_b = b.get_bounding_box();

    auto a_bottom_right = bb_a.top_left;
    a_bottom_right += bb_a.dimensions;

    auto b_bottom_right = bb_b.top_left;
    b_bottom_right += bb_b.dimensions;

    return 
        (! // both NOT cleared -> collision
        ((bb_a.top_left.x > b_bottom_right.x) || (a_bottom_right.x < bb_b.top_left.x))  // x-axis cleared
        && 
        ((bb_a.top_left.y > b_bottom_right.y) || (a_bottom_right.y < bb_b.top_left.y)) // y-axis cleared
        );
}
