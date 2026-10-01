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

        if (auto* atk = dynamic_cast<i_attacker*>(wo))
            attackers.push_back(atk);
    }

    void remove_object(world_object* wo)
    {
        std::lock_guard<std::mutex> world_guard(world_mutex);
        everything.erase(std::find(everything.begin(), everything.end(), wo));

        if (auto* plyr = dynamic_cast<player*>(wo))
            the_player = nullptr;
            
        if (auto* drw = dynamic_cast<i_drawable*>(wo))
        {
            drawables.erase(std::find(drawables.begin(), drawables.end(), drw));
        }

        if (auto* mv = dynamic_cast<i_movable*>(wo))
        {
            movables.erase(std::find(movables.begin(), movables.end(), mv));
        }

        if (auto* atk = dynamic_cast<i_attacker*>(wo))
        {
            attackers.erase(std::find(attackers.begin(), attackers.end(), atk));
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


    void spawn_projectile(coordinate position, attack_info ai)
    {
        dynamic_spawns.push_back(
            std::make_unique<projectile>(
                position,
                ai.dir
            )
        );
        add_object(dynamic_spawns.back().get());
    }


    void process_attacks()
    {
        std::lock_guard<std::mutex> dog(dynob_mutex);

        for (auto* atk : attackers)
        {
            if (atk->attack())
            {
                auto ai = atk->get_attack_info();
                for (const auto& hp : atk->get_hardpoints())
                {
                    auto spawn = ai.position;
                    spawn += hp;
                    spawn_projectile(spawn, ai);
                }
            }
        }
    }

    void scroll() // TODO: deprecate
    {
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
    	            auto y = (sy + progress % 80) % 23;
    	            dynamic_spawns.push_back(
    	            	std::make_unique<star>(
    	            		coordinate(79, (layer % 2 == 0) ? 23 - sy : sy)
    	            		, layer * milliseconds(100))
    	            		);
    	            add_object(dynamic_spawns.back().get());
    	    	}
        	} 
        }
    }

    void cleanup()
    {
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

    void on_tick(std::chrono::system_clock::duration delta_t)
    {
        static auto ctr { 0 };
        {
            std::lock_guard<std::mutex> dog(dynob_mutex);
            for (auto* o : everything) 
            {
                o->update(delta_t); 
            }
        }
        for (auto* o : movables) 
        {
            move_object(o, o->get_move_intent(), o->allow_oob()); 
        }

        process_collisions(); 
        process_attacks(); 

        cleanup();
    }
    
    void on_scroll(std::chrono::system_clock::duration delta_t)
    { // TODO: move to on_tick with scroll_timer
        //scroll(); 
        spawn_stars(3); 
        //cleanup(); 
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
            case 0:
                dynamic_spawns.push_back(
                    std::make_unique<rhombus>(
                        coordinate { 40 + x, 2 + y }
                    )
                );
            break;
            case 1:
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
            case 42:
                if (!the_player)
                {
                    dynamic_spawns.push_back(
                        std::make_unique<player>(
                            coordinate {20, 10}
                        )
                    );
                    the_player = dynamic_cast<player*>(dynamic_spawns.back().get());
                }
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
    std::vector<i_attacker*> attackers {};
    player* the_player { nullptr };

    std::size_t progress { 0 };
};
