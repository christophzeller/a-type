#pragma once

#include "../input/input.hxx"
#include "objects.hxx"
#include "enemies.hxx"
#include "player.hxx"
#include "starfield.hxx"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdlib>
#include <ctime>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include <unistd.h>

using namespace std::chrono;
using namespace std::literals::chrono_literals;

std::mutex world_mutex;

void move_object(world_object& object, direction dir, bool allow_oob=false)
{
    std::lock_guard<std::mutex> world_guard(world_mutex);

    switch(dir)
    {
    case NORTH:
        if (!allow_oob)
            object.position_.y = std::clamp( object.position_.y -= 1, 0, 23); //w N
        else
        {
            object.position_.y -= 1;
        }
        break;
    case WEST:
        if (!allow_oob)
            object.position_.x = std::clamp( object.position_.x -= 1, 0, 79); //a W
        else
        {
            object.position_.x -= 1;
        }
        break;
    case SOUTH:
        if (!allow_oob)
            object.position_.y = std::clamp( object.position_.y += 1, 0, 23); //s S
        else
        {
            object.position_.y += 1;
        }
        break;
    case EAST:
        if (!allow_oob)
            object.position_.x = std::clamp( object.position_.x += 1, 0, 79); //d E
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

struct engine_state
{
	void tick()
	{
		++current_tick;
		
		last_tick = now;
		now = system_clock::now();

		delta_t = now - last_tick;

		on_tick(delta_t);
		if (now >= next_tick)
		{
			next_tick = now + scroll_rate;
			on_scroll(delta_t);
		}
	}

    engine_state(input_listener& input) : input_(input)
    {
        input_.add_callback('q', [this](){ is_paused = false; pause_cv.notify_all(); is_running = false; });
        input_.add_callback('p', [this](){ is_paused = !is_paused; if (!is_paused) pause_cv.notify_all(); } ) ;
    }

    input_listener& input_;

    std::function<void(system_clock::duration)> on_tick;
    std::function<void(system_clock::duration)> on_scroll;
    system_clock::duration scroll_rate;
    
    time_point<system_clock> last_tick;
    time_point<system_clock> next_tick;
    time_point<system_clock> now;

    std::mutex pause_mutex;
    std::condition_variable pause_cv;

    std::atomic<bool> is_running { true };
    std::atomic<bool> is_paused { false };

    system_clock::duration render_t;

    system_clock::duration delta_t;
    std::size_t current_tick { 0 };
};
