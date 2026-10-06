#pragma once
#include "engine/objects.hxx"

#include <chrono>
#include <cmath>
#include <cstddef>
#include <string>
#include <vector>

using namespace std::chrono;
using namespace std::literals::chrono_literals;

struct announcer : public animation
{
    announcer(coordinate position) 
        : animation(position)
    {

        model_chars = 
        {

            {
                'X', 'X', 'X',
                'X', '*', 'X',
                'X', 'X', 'X'
            },
            {
                'X', '*', 'X',
                '*', '*', '*',
                'X', '*', 'X'
            },
            {
                '*', '*', '*',
                '*', 'X', '*',
                '*', '*', '*'
            },
            {
                '*', 'X', '*',
                'X', 'X', 'X',
                '*', 'X', '*'
            }
        };
    }



    std::size_t animation_index { 0 };
    coordinate model_dimensions { 3, 3 };
    system_clock::duration animation_time { 600ms };
    system_clock::duration animation_step { 150ms };
    system_clock::duration timer { 600ms };

    bool is_looping { false };
};
