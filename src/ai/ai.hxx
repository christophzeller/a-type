#pragma once

#include "../world/world.hxx"

#include <cstdlib>
#include <iostream>

using namespace std::chrono;
using namespace std::literals::chrono_literals;

direction get_direct_path(const coordinate& from, const coordinate& to)
{
    auto delta_x = from.x - to.x;
    auto delta_y = from.y - to.y;

    if ((delta_x == 0) && (delta_y == 0))
        return STATIC;
    
    if (std::abs(delta_x) > std::abs(delta_y))
    {
        if (delta_x > 0)
            return WEST;
        else
            return EAST;
    }
    else
    {
        if (delta_y > 0)
            return NORTH;
        else
            return SOUTH;
    }
}

void hunt(world_object& hunter, const world_object& target)
{
    auto* movable = dynamic_cast<i_movable*>(&hunter);

    if (!movable) return;

    movable->set_move_intent(get_direct_path(hunter.position_, target.position_));
}


void block(world_object& protector, const world_object& aggressor, const world_object& target)
{
    auto* movable = dynamic_cast<i_movable*>(&protector);

    if (!movable) return;

    auto mid_x = (aggressor.position_.x + target.position_.x) / 2;
    auto mid_y = (aggressor.position_.y + target.position_.y) / 2;

    movable->set_move_intent(get_direct_path(protector.position_, coordinate{mid_x, mid_y}));
}


void erratic(i_movable& obj, bool allow_x=true, bool allow_y=true)
{
    auto x = rand() % 8;

    switch (x)
    {
    case 0:
        if (allow_y)
            obj.set_move_intent(NORTH);
    break;
    case 1:
        if (allow_y)
            obj.set_move_intent(SOUTH);
    break;
    case 2:
        if (allow_x)
            obj.set_move_intent(EAST);
    break;
    case 3:
        if (allow_x)
            obj.set_move_intent(WEST);
    break;
    case 4:
    case 5:
    case 6:
    case 7:
        obj.set_move_intent(STATIC);
    break;
    }
}

struct asteroid : public world_object, i_movable, i_drawable
{
    asteroid(coordinate position)
    : world_object('O', position, []() {})
    , animation_index(rand() % 12)
    , next_move(std::chrono::system_clock::now()) 
    {
        /*auto x = rand() % 3;

        switch (x)
        {
        case 0:
            drift_dir = NORTH;
        break;
        case 1:
            drift_dir = SOUTH;
        break;
        case 2:
            drift_dir = STATIC;
        break;
        }*/
    }

    char get_representation() override 
    {
        return animation[animation_index++ % 8]; 
    }
    coordinate where() override { return position_; }
    
    void set_move_intent(direction dir) override { move_intent = dir; }

    direction get_move_intent(bool reset = true) 
    { 
        using namespace std::literals::chrono_literals;
        
        if (std::chrono::system_clock::now() < next_move)
            return STATIC;

        next_move = std::chrono::system_clock::now() + milliseconds(move_interval);    
        return drift_dir;
        /*if (reset) 
        {
            auto tmp = move_intent; 
            move_intent = STATIC; 
            return tmp;
        } 
        else
            return move_intent;
        */
    }

    bool allow_autoscroll() override { return true; }

    coordinate from() override { return position_; };

    std::chrono::time_point<std::chrono::system_clock> next_move;
    int move_interval { 1000 };
    char animation[12] { 'o', 'o', 'o', 'o', 'o', 'o', 'O', 'O', 'O', 'O', 'O', 'O'};
    std::size_t animation_index { 0 };
    direction move_intent { STATIC };

    direction drift_dir { STATIC };
};
