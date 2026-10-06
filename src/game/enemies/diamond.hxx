#pragma once

#include "enemies.hxx"
#include "engine/ai/ai.hxx"

struct diamond : public enemy//, world_object, i_movable, i_drawable
{
    explicit diamond(coordinate position) 
        : diamond(position, waypoints) 
    {

    }

    diamond(coordinate position, std::vector<coordinate> flight_plan)
        : enemy(position, 2s, 175ms, flight_plan) 
    {
		model_chars = { 
	        '-', '/', '\\', 'X',
	        '/', ' ', '=', '\\',
	        '\\', ' ', '=', '/',
	        '-', '\\', '/', 'X'
	        };
        model_dimensions = { 4, 4 };

        guns.clear();
        guns.push_back( {WEST, {-4, 0}} );
        guns.push_back( {WEST, {-4, 3}} );
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
    	enemy::update(delta_t);
        erratic(*this, true, false);
    }
};
