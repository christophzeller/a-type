#pragma once

#include "../input/input.hxx"
#include "objects.hxx"
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


struct player : public world_object, i_movable, i_drawable
{
    player(coordinate position) : world_object('>', position, [](){}) {}
    char get_representation() override 
    {
        return symbol_; 
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

    void on_capture()
    {
        score += 1;
    }

    //char animation[14] { '.', '.', 'o', 'o', '8', '8', 'O', 'O', '8', '8', 'o', 'o', '.', '.'};
    int score = 0;
};


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
    engine_state(input_listener& input) : input_(input)
    {
        input_.add_callback('q', [this](){ is_paused = false; pause_cv.notify_all(); is_running = false; });
        input_.add_callback('p', [this](){ is_paused = !is_paused; if (!is_paused) pause_cv.notify_all(); } ) ;
    }

    input_listener& input_;
    
    time_point<system_clock> last_tick;
    time_point<system_clock> next_tick;
    time_point<system_clock> now;

    std::mutex pause_mutex;
    std::condition_variable pause_cv;

    std::atomic<bool> is_running { true };
    std::atomic<bool> is_paused { false };
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
