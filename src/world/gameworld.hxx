#pragma once

#include "enemies.hxx"
#include "objects.hxx"
#include "player.hxx"
#include "projectile.hxx"
#include "starfield.hxx"
#include "world.hxx"

#include <algorithm>
#include <chrono>
#include <memory>
#include <mutex>
#include <vector>

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
    }

    void process_collisions()
    {
        for (auto* coll : colliders)
        {
            if (is_collision(*the_player, *coll))
            {
                the_player->on_collision(coll);
            }
        }
    }

    void process_attacks()
    {
        auto spawn = coordinate {the_player->position_};
        spawn += coordinate{3, 0};
        
        dynamic_spawns.push_back(
            std::make_unique<projectile>(
                spawn
            )
        );
        add_object(dynamic_spawns.back().get());

        spawn += coordinate{0, 2};

        dynamic_spawns.push_back(
            std::make_unique<projectile>(
                spawn
            )
        );
        add_object(dynamic_spawns.back().get());        
    }

    void scroll()
    {
       for (auto* o : movables) // TODO: autoscrolls? 
        {
            if (o->allow_autoscroll())
            {
                move_object(o, WEST, true);
                if (o->get_position().x <= 0)
                {
                    kill_list.push_back(dynamic_cast<world_object*>(o));
                }
            }
        }
    }

    void spawn_stars(std::size_t layers=1)
    {
        for (auto layer = 1; layer <= layers; ++layer)
        {
        	auto step = (layer - 1) ? layer * 4 : 1;
        	if (progress % step == 0)
        	{
    	    	const auto& star_list = star_loop[(progress / step )% 80];
    	        for (auto sy : star_list)
    	        {
    	        	// TODO: fiddle with coordinate manipulation
    	            stars.push_back(
    	            	std::make_unique<star>(
    	            		coordinate(79, (layer % 2 == 0) ? 23 - sy : sy)
    	            		, (layer == 1) ? '.' : '.'
    	            		, layer));
    	            add_object(stars.back().get());
    	    	}
        	} 
        }
    }

    void cleanup()
    {
        for (auto* o : kill_list)
        {
            remove_object(o);
            if (auto s = dynamic_cast<star*>(o))
            {
                for (auto up = stars.begin(); up != stars.end(); ++up)
                {
                    if (up->get() == o)
                    {
                        stars.erase(up);
                        break;
                    }
                }
            }
        }
        kill_list.clear();
    }

    void on_tick(std::chrono::system_clock::duration delta_t)
    {
        for (auto* o : everything) 
            o->update(delta_t); 
        for (auto* o : movables) 
            move_object(o, o->get_move_intent(), o->allow_oob()); 

        process_collisions(); 
//        process_attacks(); 
    }
    
    void on_scroll(std::chrono::system_clock::duration delta_t)
    {
        scroll(); 
        spawn_stars(3); 
        cleanup(); 
        progress += 1; 
    }

    std::vector<std::unique_ptr<world_object>> dynamic_spawns;
    std::vector<std::unique_ptr<star>> stars;

    std::vector<world_object*> kill_list;
    std::vector<world_object*> everything;
    std::vector<i_drawable*> drawables;
    std::vector<i_movable*> movables;
    std::vector<i_collider*> colliders;
    std::vector<enemy*> enemies;
    player* the_player;

    std::size_t progress { 0 };
};
