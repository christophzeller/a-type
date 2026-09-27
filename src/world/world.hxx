#pragma once

#include "../input/input.hxx"
#include "objects.hxx"
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

bool is_collision(const world_object& a, const world_object& b)
{
    std::lock_guard<std::mutex> world_guard(world_mutex);
    return a.position_.x == b.position_.x && a.position_.y == b.position_.y;
}

void move_object(world_object& object, direction dir, bool allow_oob=false)
{
    std::lock_guard<std::mutex> world_guard(world_mutex);

    switch(dir)
    {
    case NORTH:
        object.position_.y = std::clamp( object.position_.y -= 1, 0, 23); //w N
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
        object.position_.y = std::clamp( object.position_.y += 1, 0, 23); //s S
        break;
    case EAST:
        object.position_.x = std::clamp( object.position_.x += 1, 0, 79); //d E
        break;
    }
}

void move_object(i_movable* movable, direction dir, bool allow_oob=false)
{
    if (movable->move())
        move_object(*dynamic_cast<world_object*>(movable), dir);
}

struct game_world
{
    void add_object(world_object* wo)
    {
        std::lock_guard<std::mutex> world_guard(world_mutex);
        everything.push_back(wo);
    
        if (auto* drw = dynamic_cast<i_drawable*>(wo))
            drawables.push_back(drw);

        if (auto* plyr = dynamic_cast<player*>(wo))
            the_player = plyr;

        if (auto* mv = dynamic_cast<i_movable*>(wo))
            movables.push_back(mv);
    }

    void remove_object(world_object* wo)
    {
        std::lock_guard<std::mutex> world_guard(world_mutex);
        everything.erase(std::find(everything.begin(), everything.end(), wo));
        
        std::remove(everything.begin(), everything.end(), wo);

        if (auto* drw = dynamic_cast<i_drawable*>(wo))
        {
            drawables.erase(std::find(drawables.begin(), drawables.end(), drw));
        }

        if (auto* mv = dynamic_cast<i_movable*>(wo))
        {
            movables.erase(std::find(movables.begin(), movables.end(), mv));
        }
    }

    std::vector<world_object*> everything;
    std::vector<i_drawable*> drawables;
    std::vector<i_movable*> movables;
    player* the_player;

    std::size_t progress { 0 };
};

// game_state ?
struct engine_state
{
	void tick()
	{
		++current_tick;
		
		last_tick = now;
		now = system_clock::now();

		delta_t = now - last_tick;
//		std::cout << "                                                              dt: " << delta_t.count() << "\n";

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

    system_clock::duration delta_t;
    std::size_t current_tick { 0 };
};

struct i_autoscroller
{
    virtual std::size_t get_progress() = 0;
    virtual std::size_t get_speed() = 0;
};

struct my_game : public i_autoscroller
{
    std::size_t get_progress() override { return progress; }
    std::size_t get_speed() override { return 8; }

    std::size_t progress { 0 };
};
