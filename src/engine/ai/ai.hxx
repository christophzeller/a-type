#pragma once

#include "engine/objects.hxx"

#include <cstdlib>
#include <iostream>

using namespace std::chrono;
using namespace std::literals::chrono_literals;
using namespace engine;

namespace engine::ai
{

direction get_direct_path(const coordinate& from, const coordinate& to)
{
    auto delta_x = from.x - to.x;
    auto delta_y = from.y - to.y;

    if ((delta_x == 0) && (delta_y == 0))
        return STATIC;

	if (delta_y == 0)
	{
        if (delta_x > 0)
            return WEST;
        else
            return EAST;
	}

	if ((delta_x % delta_y) == 0)
	{
        if (delta_y > 0)
            return NORTH;
        else
            return SOUTH;
	}

    if (delta_x > 0)
        return WEST;
    else
        return EAST;
}

void navigate(world_object& mover, const coordinate& destination)
{
    auto* movable = dynamic_cast<i_movable*>(&mover);
    if (!movable) 
        return;

    movable->set_move_intent(get_direct_path(mover.position_, destination));
}

void navigate(world_object& mover, const world_object& target)
{
	navigate(mover, target.position_);
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

} // engine::ai
