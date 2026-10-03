#pragma once

#include "engine/objects.hxx"
#include "engine/utilities.hxx"

#include <chrono>

using namespace std::chrono;
using namespace std::literals::chrono_literals;
using namespace engine::utilities;


struct player : public world_object, i_movable, i_drawable, i_collider, i_attacker
{
    player(coordinate position) : world_object('>', position, [](){}) 
    {
        guns.push_back( coordinate{4, 0} );
        guns.push_back( coordinate{4, 2} );
    }

    // i_drawable
    render_info get_render_info() override 
    {
        bounding_box bb;
        bb.top_left = position_;
        bb.dimensions = model_dimensions;
        
        return render_info{ bb, model_chars, 'X', 99 };
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
        if (reset) 
        { 
            auto tmp = move_intent; 
            move_intent = STATIC; 
            return tmp;
        } 
    	else 
        	return move_intent; 
	}

	coordinate get_position() override { return position_; }

    direction move_intent { STATIC };

    bool move() override
    {
        if (move_timer < 0ms)
        {
            move_timer = speed;
            return true;
        }
        return false;
    }

    bool allow_oob() const override { return false; }

    extents get_extents() const
    {
        return engine::utilities::get_extents(get_bounding_box());
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
        '|','\\','-',
        '|',' ','D',
        '|','/','-' 
        };

    bool attack() override
    {
        if (attack_intent)
        {
            attack_intent = false;
            return true;
        }
        return attack_intent;
    }

    attack_info get_attack_info() const override
    {
        return attack_info { EAST, coordinate{position_} };
    }

    const hardpoints& get_hardpoints() const override
    {
        return guns;
    }

    std::atomic<bool> attack_intent { false };

    hardpoints guns { };
    coordinate model_dimensions { 3, 3 };
    system_clock::duration move_timer { 50ms };
    system_clock::duration speed { 50ms };
};
