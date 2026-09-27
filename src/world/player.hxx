#pragma once

#include "objects.hxx"

#include <chrono>
#include <vector>

using namespace std::chrono;
using namespace std::literals::chrono_literals;

struct player : public world_object, i_movable, i_drawable
{
    player(coordinate position) : world_object('>', position, [](){}) {}
    char get_representation() override 
    {
        return symbol_; 
    }

    render_info get_render_info() override 
    {
        bounding_box bb;
        bb.top_left = position_;
        bb.dimensions = model_dimensions;
        return render_info{ bb, model_chars };
    }

    void update(system_clock::duration delta_t) override
    {
        if (move_intent != STATIC)
        {
            move_timer -= delta_t;
        }
    }

    coordinate where() override { return position_; }

    void set_move_intent(direction dir) override 
    {
    	if (dir == move_intent)
    		move_intent = STATIC;
		else
	    	move_intent = dir; 
    }
    
    direction get_move_intent(bool reset = false) override 
    { 
        if (reset) 
        { 
            auto tmp = move_intent; 
            move_intent = STATIC; 
            return tmp;
        } 
    	else 
        	return move_intent; 
	}

	bool allow_autoscroll() override { return false; }

	coordinate from() override { return position_; }

    direction move_intent { STATIC };

    std::chrono::system_clock::duration get_speed() override { using namespace std::literals::chrono_literals; return 50ms; }
    bool move() override
    {
        if (move_timer < 0ms)
        {
            move_timer = speed;
            return true;
        }
        return false;
    }

    std::vector<char> model_chars { 
        '|','\\','-',
        '|',' ','D',
        '|','/','-' 
        };
    coordinate model_dimensions { 3, 3 };
    system_clock::duration move_timer { 50ms };
    system_clock::duration speed { 50ms };
};
