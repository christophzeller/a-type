#pragma once


#include "engine/objects.hxx"
#include "engine/utilities.hxx"

#include "enemies/rhombus.hxx"
#include "enemies/torus.hxx"
#include "enemies/diamond.hxx"

#include "game/player.hxx"
#include "game/projectile.hxx"
#include "game/starfield.hxx"

#include "game/level.hxx"

#include <algorithm>
#include <chrono>
#include <memory>
#include <mutex>
#include <vector>

#include <cstddef>

using namespace engine;
using namespace engine::utilities;

std::mutex world_mutex;

struct game_world // : public i_game 
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
        
        auto collisions = get_collisions(everything, get_extents(world_bb));
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

    void spawn_stars(std::size_t layers)
    {
        std::lock_guard<std::mutex> dog(dynob_mutex);
        for (auto layer = 1; layer <= layers; ++layer)
        {
        	auto step = (layer - 1) ? layer * 4 : 1;
        	if (progress % step == 0)
        	{
    	    	const auto& star_list = star_loop[(progress / step ) % 80];
    	        for (auto sy : star_list)
    	        {
    	            // spawn at right camera view edge
//    	            auto y = (sy + progress % screen_dimensions.width) % (screen_dimensions.height - 1);
//    	            (layer % 2 == 0) ? y = 23 - y : y;
    	            dynamic_spawns.push_back(
    	            	std::make_unique<star>(
    	            		coordinate(
    	            		    camera_viewport.dimensions.x - 1, 
    	            		    //sy
    	            		    (layer % 2 == 0) ? camera_viewport.dimensions.y - 1 - sy : sy
    	            		    )
    	            		, layer * milliseconds(100))
    	            		);
    	            add_object(dynamic_spawns.back().get());
    	    	}
        	} 
        }
    }

    void progress_level()
    {
        std::lock_guard<std::mutex> dog(dynob_mutex);

        if (!level)
            return;

        if (l1.spawns.count(progress) > 0)
        {
            for (const auto& si : l1.spawns[progress])
            {
                dynamic_spawns.push_back( 
                    std::make_unique<rhombus>( si.location, si.waypoints )
                );
                add_object(dynamic_spawns.back().get());
            }
            l1.spawns.erase(progress);
        }
    }

    void update_objects(system_clock::duration delta_t)
    {
        std::lock_guard<std::mutex> dog(dynob_mutex);
        for (auto* o : everything) 
        {
            o->update(delta_t); 
        }
    }

    void move_objects()
    {
        std::lock_guard<std::mutex> wog(world_mutex);
        for (auto* o : movables) 
        {
            move_object(o, o->get_move_intent(), get_extents(world_bb), o->allow_oob()); 
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
        
            if ( is_oob( it->get()->position_, get_extents(world_bb) ) )
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

    void check_scroll(system_clock::duration delta_t)
    {
        scroll_timer -= delta_t;
        if (scroll_timer < 0ms)
        {
            scroll_timer = scroll_interval;
            spawn_stars(3);

            ++progress;
        }
    }

    void on_tick(std::chrono::system_clock::duration delta_t, engine_state es)
    {
        static auto ctr { 0 };

        check_scroll(delta_t);
        progress_level();

        update_objects(delta_t);
        move_objects();

        process_collisions(); 
        process_attacks(); 

        cleanup();
    }


    void spawn(std::size_t what, coordinate where, std::vector<coordinate> flight_plan)
    {
        std::lock_guard<std::mutex> dog(dynob_mutex);

        switch (what)
        {
            case 0:
                dynamic_spawns.push_back(
                    std::make_unique<rhombus>(
                        where,
                        flight_plan
                    )
                );
            break;
/*            case 1:
                dynamic_spawns.push_back(
                    std::make_unique<torus>(
                        where,
                        flight_plan
                    )
                );     
            break;
            case 2:
                dynamic_spawns.push_back(
                    std::make_unique<diamond>(
                        where,
                        flight_plan
                    )
                );*/
            break;
            default:
                return;
        }
        add_object(dynamic_spawns.back().get());
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
                else
                {
                    return;
                }
            break;
        }   
        add_object(dynamic_spawns.back().get());
    }


    level1 l1 {};

    std::size_t level { 0 };

    std::mutex dynob_mutex {};

    std::vector<std::unique_ptr<world_object>> dynamic_spawns {};
    std::vector<std::unique_ptr<star>> stars {};

    std::vector<world_object*> kill_list {};
    std::vector<world_object*> everything {};
    std::vector<i_drawable*> drawables {};
    std::vector<i_movable*> movables {};
    std::vector<i_attacker*> attackers {};
    player* the_player { nullptr };

    system_clock::duration scroll_interval = 100ms;
    system_clock::duration scroll_timer = 100ms;

    // size should be identical to terminal screen dimensions
    bounding_box camera_viewport
     {
        coordinate {0, 0},
        coordinate {80, 24}
     };

    bounding_box world_bb
    {
        coordinate {0, 0},
        coordinate {80, 24}
    };
    std::size_t progress { 0 };
};
