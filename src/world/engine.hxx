#pragma once

#include "../input/input.hxx"
#include "objects.hxx"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdlib>
#include <ctime>
#include <functional>
#include <mutex>

#include <unistd.h>

using namespace std::chrono;
using namespace std::literals::chrono_literals;

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
