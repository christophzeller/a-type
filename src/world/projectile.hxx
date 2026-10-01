#pragma once

#include "objects.hxx"

#include <chrono>
#include <vector>

using namespace std::chrono;
using namespace std::literals::chrono_literals;

struct projectile : public world_object, i_movable, i_drawable, i_collider
{
    // move_speed
    projectile(coordinate position, direction dir=EAST) : world_object('~', position, [](){}), move_intent(dir) {}
    ~projectile() = default;

    // i_drawable
    render_info get_render_info() override 
    {
        bounding_box bb;
        bb.top_left = position_;
        bb.dimensions = model_dimensions;
        
        return render_info{ bb, model_chars, 'X', 80 };
    }

    // world_object
    void update(system_clock::duration delta_t) override
    {
        if (move_intent != STATIC)
        {
            move_timer -= delta_t;
        }
    }

    // i_movable
    void set_move_intent(direction dir) override 
    {
    	if (dir == move_intent)
    		move_intent = STATIC;
		else
	    	move_intent = dir; 
    }
    
    direction get_move_intent(bool reset = false) override 
    { 
    	return move_intent; 
	}

	coordinate get_position() override { return position_; }

    direction move_intent { EAST };

    bool move() override
    {
        if (move_timer < 0ms)
        {
            move_timer = speed;
            return true;
        }
        return false;
    }

    bool allow_oob() const override { return true; }

    extents get_extents() const
    {
        return ::get_extents(get_bounding_box());
    }

    // i_collider
    bounding_box get_bounding_box() const 
    {
        bounding_box bb;
        bb.top_left = coordinate{position_};
        bb.dimensions = coordinate{model_dimensions};

        return bb;  // TODO: member & update on move?
    }
    
    void on_collision(i_collider* other) override 
    {
    }

    std::vector<char> model_chars { 
        '~', '~', '~'
        };
    coordinate model_dimensions { 3, 1 };
    system_clock::duration move_timer { 10ms };
    system_clock::duration speed { 10ms };
};
