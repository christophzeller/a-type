#pragma once

#include "objects.hxx"

#include <cstdlib>
#include <set>
#include <vector>

extents get_extents(const bounding_box bb)
{
    return (extents {
        bb.top_left,
        coordinate { bb.top_left.x + bb.dimensions.x - 1, 
                    bb.top_left.y + bb.dimensions.y - 1 }
    });
}

coordinate get_random_coord()
{
    auto x = rand() % 79;
    auto y = rand() % 23;

    return {x, y};
}

// TODO: add ", extents borders)" argument
bool is_oob(coordinate c)
{
    if (c.x < 0 || c.x > 79 || c.y < 0 || c.y > 23)
        return true;

    return false;
}

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

            if (is_oob(lhs->position_))
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

// TODO: add ", extents borders)" argument
void move_object(world_object& object, direction dir, bool allow_oob=false)
{
    bool oob { false };
    auto ext = object.get_extents();
    if (is_oob(ext.minima) || is_oob(ext.maxima))
        oob = true;

    auto x_offset = ext.maxima.x - ext.minima.x;
    auto y_offset = ext.maxima.y - ext.minima.y;

    switch(dir)
    {
    case NORTH:
        if (!allow_oob)
        {
            if (!oob)
                object.position_.y = std::clamp( object.position_.y -= 1, 0, 23 ); //w N
        }
        else
        {
            object.position_.y -= 1;
        }
        break;
    case WEST:
        if (!allow_oob)
        {
            if (!oob)
               object.position_.x = std::clamp( object.position_.x -= 1, 0, 79); //a W
        }
        else
        {
            object.position_.x -= 1;
        }
        break;
    case SOUTH:
        if (!allow_oob)
        {
            if (!oob)
                object.position_.y = std::clamp( object.position_.y += 1, 0, 23 - y_offset); //s S
        }
        else
        {
            object.position_.y += 1;
        }
        break;
    case EAST:
        if (!allow_oob)
        {
            if (!oob)
                object.position_.x = std::clamp( object.position_.x += 1, 0, 79 - x_offset); //d E
        }
        else
        {
            object.position_.x += 1;
        }
        break;
    }
}

void move_object(i_movable* movable, direction dir, bool allow_oob=false)
{
    if (movable->move())
        move_object(*dynamic_cast<world_object*>(movable), dir, allow_oob);
}
