#pragma once

#include "enemies.hxx"
#include "rhombus.hxx"
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
    }

    void remove_object(world_object* wo)
    {
        std::lock_guard<std::mutex> world_guard(world_mutex);
        everything.erase(std::find(everything.begin(), everything.end(), wo));
        
        if (auto* drw = dynamic_cast<i_drawable*>(wo))
        {
            drawables.erase(std::find(drawables.begin(), drawables.end(), drw));
        }

        if (auto* mv = dynamic_cast<i_movable*>(wo))
        {
            movables.erase(std::find(movables.begin(), movables.end(), mv));
        }
    }

    void process_collisions()
    {
        std::lock_guard<std::mutex> world_guard(world_mutex);
        
        auto collisions = get_collisions(everything);
        for (const auto& c : collisions)
        {
            for (auto o : c)
            {
                o->position_ = { -100, -100 };
            }
        }
    }

    void process_attacks()
    {
        std::lock_guard<std::mutex> dog(dynob_mutex);
        if (the_player->attack())
        {
            auto spawn = coordinate {the_player->position_};
            spawn += coordinate{4, 0};

            dynamic_spawns.push_back(
                std::make_unique<projectile>(
                    spawn
                )
            );
            add_object(dynamic_spawns.back().get());
        }

        for (auto& o : dynamic_spawns)
        {
            if (auto* nmy = dynamic_cast<enemy*>( o.get() ) ) 
            {
                if (nmy->attack())
                {
                    auto spawn = coordinate {o->position_};
                    spawn += coordinate{-5, 0};

                    dynamic_spawns.push_back(
                        std::make_unique<projectile>(
                            spawn,
                            WEST
                        )
                    );
                    add_object(dynamic_spawns.back().get());
                }
            }
        }
    }

    void scroll()
    {
       for (auto* o : movables) // TODO: autoscrolls? 
        {
            if (o->allow_autoscroll())
            {
                move_object(o, WEST, true);
                if (is_oob(o->get_position()))
                {
                    kill_list.push_back(dynamic_cast<world_object*>(o));
                }
            }
        }
    }

    void spawn_stars(std::size_t layers=1)
    {
        std::lock_guard<std::mutex> dog(dynob_mutex);
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
        std::lock_guard<std::mutex> dog(dynob_mutex);
        for (auto* o : kill_list)
        {
            remove_object(o);
            if (auto* s = dynamic_cast<star*>(o))
            {
                for (auto up = stars.begin(); up != stars.end(); )
                {
                    if (up->get() == o)
                    {
                        up = stars.erase(up);
                        break;
                    }
                    else
                    {
                        ++up;
                    }
                }
            }
        }
        kill_list.clear();

        
    }

    void on_tick(std::chrono::system_clock::duration delta_t)
    {
        static auto ctr { 0 };
        for (auto* o : everything) 
        {
            o->update(delta_t); 
        }
        for (auto* o : movables) 
        {
            move_object(o, o->get_move_intent(), o->allow_oob()); 
        }

        process_collisions(); 
        process_attacks(); 

        std::lock_guard<std::mutex> dog(dynob_mutex);
        for (auto it = dynamic_spawns.begin(); it != dynamic_spawns.end(); )
        {
            if (it->get() == nullptr)
            {
                std::cin.get();
            }
        
            if ( is_oob( it->get()->position_ ) )
            {
                remove_object(it->get());
                it = dynamic_spawns.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }
    
    void on_scroll(std::chrono::system_clock::duration delta_t)
    {
        scroll(); 
        spawn_stars(3); 
        cleanup(); 
        progress += 1; 
    }

    void spawn(int type = -1)
    {
        std::lock_guard<std::mutex> dog(dynob_mutex);
        auto type_ = type;
        if (type_ == -1)
            type_ = rand() % 3;
            
        auto x = rand() % 35;
        auto y = rand() % 16;

        switch (type_)
        {
            case 1:
            dynamic_spawns.push_back(
                std::make_unique<rhombus>(
                    coordinate { 40 + x, 2 + y }
                )
            );
            break;
            case 0:
            dynamic_spawns.push_back(
                std::make_unique<torus>(
                    coordinate { 40 + x, 2 + y }
                )
            );     
            break;
            case 2:
            dynamic_spawns.push_back(
                std::make_unique<diamond>(
                    coordinate { 40 + x, 2 + y }
                )
            );
            break;
        }   
        add_object(dynamic_spawns.back().get());
    }

    void despawn()
    {
        
    }

    std::mutex dynob_mutex {};

    std::vector<std::unique_ptr<world_object>> dynamic_spawns {};
    std::vector<std::unique_ptr<star>> stars {};

    std::vector<world_object*> kill_list {};
    std::vector<world_object*> everything {};
    std::vector<i_drawable*> drawables {};
    std::vector<i_movable*> movables {};
    player* the_player { nullptr };

    std::size_t progress { 0 };
};
