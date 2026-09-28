#pragma once

#include "../ai/ai.hxx"
#include "objects.hxx"

#include <chrono>
#include <vector>

using namespace std::chrono;
using namespace std::literals::chrono_literals;

// TODO: 
// AI movement: navigate to a set of waypoints. possible presets:
// current player position once, then continue off-screen (aimed non-homing collision course)
// current player position continuous (collision course)
// current player position for a set amount of time (time-bound homing)
// diagonal across the screen
// z-pattern across the screen
// circle around a point
// avoid player line of fire
// hold position relative to an object (formation)
// hold position relative to two objects (block)
// 

struct enemy : public world_object, i_movable, i_drawable
{
    enemy(coordinate position) : world_object('?', position, [](){} ) {}
    void update(system_clock::duration delta_t) override { logic_(); }

    char get_representation() override { return '?'; }
    render_info get_render_info() override { return render_info(); }

    void set_move_intent(direction dir) override {}
    direction get_move_intent(bool reset=false) override { return STATIC; }
    bool allow_autoscroll() override { return false; }
    coordinate get_position() override { return coordinate(); }
    bool move() override { return false; }
    std::chrono::system_clock::duration get_speed() override { return 0ms; }
};

struct torus : public enemy, i_collider //, world_object, i_movable, i_drawable
{
    torus(coordinate position) : enemy(position) {} //world_object('#', position, [](){}) {}
    char get_representation() override 
    {
        return symbol_; 
    }

    render_info get_render_info() override 
    {
        bounding_box bb;
        bb.top_left = position_;
        bb.dimensions = model_dimensions;
        
        return render_info{ bb, model_chars, model_chars[6], 50 };
    }

    void update(system_clock::duration delta_t) override
    {
        erratic(*this, false, true);
        if (move_intent != STATIC)
        {
            move_timer -= delta_t;
        }
    }

    void set_move_intent(direction dir) override 
    {
    	if (dir == move_intent)
    		move_intent = STATIC;
		else
	    	move_intent = dir; 

	    // todo: sync time
    }
    
    direction get_move_intent(bool reset = true) override 
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

	coordinate get_position() override { return position_; }

    direction move_intent { STATIC };

    std::chrono::system_clock::duration get_speed() override { using namespace std::literals::chrono_literals; return 250ms; }
    bool move() override
    {
        if (move_timer < 0ms)
        {
            move_timer = speed;
            return true;
        }
        return false;
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
        std::cout << "                                                                  COLLIE\n";
    }

    std::vector<char> model_chars { 
        '/', '=', '=', '\\',
        '|', 'X', 'X', '|',
        '\\', '=', '=', '/'
        };
    coordinate model_dimensions { 4, 3 };
    system_clock::duration move_timer { 250ms };
    system_clock::duration speed { 250ms };
};


struct rhombus : public enemy//, world_object, i_movable, i_drawable
{
    rhombus(coordinate position) : enemy(position) {} //world_object('<', position, [](){}) {}
    char get_representation() override 
    {
        return symbol_; 
    }

    render_info get_render_info() override 
    {
        bounding_box bb;
        bb.top_left = position_;
        bb.dimensions = model_dimensions;

        return render_info{ bb, model_chars, 'X', 40 };
    }

    void update(system_clock::duration delta_t) override
    {
        erratic(*this);
        if (move_intent != STATIC)
        {
            move_timer -= delta_t;
        }
    }

    void set_move_intent(direction dir) override 
    {
    	if (dir == move_intent)
    		move_intent = STATIC;
		else
	    	move_intent = dir; 

	    // todo: sync time
    }
    
    direction get_move_intent(bool reset = true) override 
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

	coordinate get_position() override { return position_; }

    direction move_intent { STATIC };

    std::chrono::system_clock::duration get_speed() override { using namespace std::literals::chrono_literals; return 125ms; }
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
        '/', '=', '\\',
        '\\', '=', '/'
        };
    coordinate model_dimensions { 3, 2 };
    system_clock::duration move_timer { 125ms };
    system_clock::duration speed { 125ms };
};


struct diamond : public enemy//, world_object, i_movable, i_drawable
{
    diamond(coordinate position) : enemy(position) {} // world_object('|', position, [](){}) {}
    char get_representation() override 
    {
        return symbol_; 
    }

    render_info get_render_info() override 
    {
        bounding_box bb;
        bb.top_left = position_;
        bb.dimensions = model_dimensions;

        return render_info{ bb, model_chars, model_chars[3], 60 };
    }

    void update(system_clock::duration delta_t) override
    {
        erratic(*this, true, false);
        if (move_intent != STATIC)
        {
            move_timer -= delta_t;
        }
    }

    void set_move_intent(direction dir) override 
    {
    	if (dir == move_intent)
    		move_intent = STATIC;
		else
	    	move_intent = dir; 

	    // todo: sync time
    }
    
    direction get_move_intent(bool reset = true) override 
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

	coordinate get_position() override { return position_; }

    direction move_intent { STATIC };

    std::chrono::system_clock::duration get_speed() override { using namespace std::literals::chrono_literals; return 175ms; }
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
        '-', '/', '\\', 'X',
        '/', ' ', '=', '\\',
        '\\', ' ', '=', '/',
        '-', '\\', '/', 'X'
        };
    coordinate model_dimensions { 4, 4 };
    system_clock::duration move_timer { 175ms };
    system_clock::duration speed { 175ms };
};
