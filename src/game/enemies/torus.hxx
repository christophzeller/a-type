#pragma once

#include "enemies.hxx"

#include "engine/ai/ai.hxx"

struct torus : public enemy // world_object, i_movable, i_drawable
{
    torus(coordinate position) : enemy(position) 
    {
	     model_chars = { 
	        '/', '=', '=', '\\',
	        '|', 'X', 'X', '|',
	        '\\', '=', '=', '/'
	        };

    	model_dimensions = { 4, 3 };
    	guns.clear();
    	guns.push_back({ WEST, {-4, 1}} );
    	guns.push_back({ EAST, { 7, 1}} );
    	guns.push_back({ NORTH, {1, -4}} );
    	guns.push_back({ SOUTH, {1, 7}} );
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
        enemy::update(delta_t);
        erratic(*this, false, true);
    }
};


