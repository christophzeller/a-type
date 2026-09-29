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

#if 0
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

        if (auto* nm = dynamic_cast<enemy*>(wo))
            enemies.push_back(nm);

        if (auto* coll = dynamic_cast<i_collider*>(wo))
            colliders.push_back(coll);        

        if (auto* pro = dynamic_cast<projectile*>(wo))
            projectiles.push_back(pro);
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

        if (auto* nm = dynamic_cast<enemy*>(wo))
        {
            enemies.erase(std::find(enemies.begin(), enemies.end(), nm));
        }

        if (auto* coll = dynamic_cast<i_collider*>(wo))
        {
            colliders.erase(std::find(colliders.begin(), colliders.end(), coll));
        }

        if (auto* pro = dynamic_cast<projectile*>(wo))
        {
            projectiles.erase(std::find(projectiles.begin(), projectiles.end(), pro));
        }
    }

    void process_collisions()
    {
        for (auto* coll : colliders)
        {
            if (is_collision(*the_player, *coll))
            {
                the_player->on_collision(coll);
                remove_object(dynamic_cast<world_object*>(coll));
            }

            for (auto* pro : projectiles)
            {
                if (is_collision(*pro, *coll))
                {
                    remove_object(dynamic_cast<world_object*>(coll));
                }
            }
        }
    }

    void process_attacks()
    {
        dynamic_spawns.push_back(
            std::make_unique<projectile>(
                coordinate {0, 20}
            )
        );
        add_object(dynamic_spawns.back().get());
    }

    std::vector<std::unique_ptr<world_object>> dynamic_spawns;

    std::vector<world_object*> everything;
    std::vector<i_drawable*> drawables;
    std::vector<i_movable*> movables;
    std::vector<i_collider*> colliders;
    std::vector<enemy*> enemies;
    std::vector<projectile*> projectiles;
    player* the_player;

    std::size_t progress { 0 };
};

#endif

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
