#pragma once
#include "engine/objects.hxx"

#include <chrono>
#include <cmath>
#include <cstddef>
#include <string>
#include <vector>

using namespace std::chrono;
using namespace std::literals::chrono_literals;

struct animation : public world_object, i_timer, i_drawable
{
    animation(coordinate position, bool looping=false) 
        : world_object(position)
        , is_looping(looping)
    {
    }

    void update(system_clock::duration delta_t) override 
    {
        /*auto idx = static_cast<int>(timer.count() % 1000);
        animation_index = std::clamp(idx, 0, 2);*/
        timer -= delta_t;

        if (timer >= 450ms)
            animation_index = 3;
        else if (timer >= 300ms)
            animation_index = 2;
        else if (timer >= 150ms)
            animation_index = 1;
        else if (timer > 0ms)
            animation_index = 0;
        else if (is_looping)
            timer = animation_time;
    }
    
    extents get_extents() const
    {
        return engine::utilities::get_extents(get_bounding_box());
    }

    bounding_box get_bounding_box() const 
    {
        bounding_box bb;
        bb.top_left = coordinate{position_};
        bb.dimensions = coordinate{model_dimensions};

        return bb;  // TODO: member & update on move?
    }

    
    bool is_expired() const override 
    {
        if (is_looping)
            return false;
        else
            return timer < 0ms;
    }

    render_info get_render_info() override 
    {
        bounding_box bb;
        bb.top_left = position_;
        bb.dimensions = model_dimensions;
        
        return render_info{ bb, model_chars[animation_index], 'X', 75, true };
    }

    std::vector<std::vector<char>> model_chars = 
    {
        {
            '*', 'X', '*',
            'X', 'X', 'X',
            '*', 'X', '*'
        },
        
        {
            '*', '*', '*',
            '*', 'X', '*',
            '*', '*', '*'
        },

        {
            'X', '*', 'X',
            '*', '*', '*',
            'X', '*', 'X'
        },

        {
            'X', 'X', 'X',
            'X', '*', 'X',
            'X', 'X', 'X'
        }

    };

    std::size_t animation_index { 0 };
    coordinate model_dimensions { 3, 3 };
    system_clock::duration animation_time { 600ms };
    system_clock::duration animation_step { 150ms };
    system_clock::duration timer { 600ms };

    bool is_looping { false };
};
