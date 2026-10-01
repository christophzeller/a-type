#pragma once

#include <chrono>
#include <functional>
#include <iostream>
#include <set>
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

    bool operator==(const coordinate& other)
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

coordinate get_random_coord()
{
    auto x = rand() % 79;
    auto y = rand() % 23;

    return {x, y};
}

bool is_oob(coordinate c)
{
    if (c.x < 0 || c.x > 79 || c.y < 0 || c.y > 23)
        return true;

    return false;
}

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

    char symbol_ { '?' }; // TODO: deprecate
    coordinate position_;
    std::function<void(void)> logic_; // TODO: void(duration) ? 
    // TODO: callbacks?

    static std::size_t instance_counter;
    //std::size_t instance_id { instance_counter++ };
};

std::size_t world_object::instance_counter = 0;

struct i_movable
{
    virtual void set_move_intent(direction dir) = 0;
    virtual direction get_move_intent(bool reset=false) = 0; // const?
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
};

struct i_drawable
{
    virtual render_info get_render_info()= 0; // const
};

struct i_collider
{
    virtual bounding_box get_bounding_box() const = 0;
    virtual void on_collision(i_collider* other) = 0;
};

bool is_collision(const i_collider& a, const i_collider& b)
{
    if (&a == &b)
        return false;

    const auto& bb_a = a.get_bounding_box();
    const auto& bb_b = b.get_bounding_box();

    coordinate tmp_a = bb_a.top_left;
    tmp_a.x += bb_a.dimensions.x - 1;
    tmp_a.y += bb_a.dimensions.y - 1;

    coordinate tmp_b = bb_b.top_left;
    tmp_b.x += bb_b.dimensions.x - 1;
    tmp_b.y += bb_b.dimensions.y - 1;
    
    return 
    !(
        ((bb_a.top_left.x > tmp_b.x) || (tmp_a.x < bb_b.top_left.x)) 
    || 
        ((bb_a.top_left.y > tmp_b.y) || (tmp_a.y < bb_b.top_left.y))
    );
}

bool is_collision(world_object* a, world_object* b)
{
    if (auto colla = dynamic_cast<i_collider*>(a))
        if (auto collb = dynamic_cast<i_collider*>(b))
            return is_collision(*colla, *collb);
    return false;
}

using collision = std::set<world_object*>;
using collision_list = std::set<collision>;

collision_list get_collisions(const std::vector<world_object*> objects) 
{
    collision_list collisions {};

    for (auto* lhs : objects)
    {
        if (!dynamic_cast<i_collider*>(lhs))
            continue;
            
        for (auto* rhs : objects)
        {
            if (lhs == rhs)
                continue;

            if (!dynamic_cast<i_collider*>(rhs))
                continue;

            if (is_oob(rhs->position_))
                continue;

            if (is_collision(lhs, rhs))
            {
                collision c;
                c.insert(lhs);
                c.insert(rhs);

                if (collisions.count(c) == 0)
                    collisions.insert(c);
            }
        }
    }

    return collisions;
}
