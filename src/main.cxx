#include "ai/ai.hxx"
#include "input/input.hxx"
#include "terminal/terminal.hxx"
#include "world/gameworld.hxx"
#include "world/projectile.hxx"
#include "world/starfield.hxx"
#include "world/enemies.hxx"
#include "world/player.hxx"
#include "world/world.hxx"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include <fcntl.h>
#include <sys/select.h>
#include <termios.h>
#include <unistd.h>

using namespace std::chrono;

void render_loop(terminal_screen& term, engine_state& es)
{
    using namespace std::literals::chrono_literals;

    std::map<std::size_t, std::vector<world_object*>> activation_list;
    
    while (es.is_running)
    {
        term.draw();
    }
}

void scroll(game_world& gw, std::vector<world_object*>& kill_list)
{
   for (auto* o : gw.movables)
    {
        if (o->allow_autoscroll())
        {
            move_object(o, WEST, true);
            if (o->get_position().x == 0)
            {
                kill_list.push_back(dynamic_cast<world_object*>(o));
            }
        }
    }
}

void spawn_stars(game_world& gw, std::vector<std::unique_ptr<star>>& stars, std::size_t layers=1)
{
    for (auto layer = 1; layer <= layers; ++layer)
    {
    	auto step = (layer - 1) ? layer * 4 : 1;
    	if (gw.progress % step == 0)
    	{
	    	const auto& star_list = star_loop[(gw.progress / step )% 80];
	        for (auto sy : star_list)
	        {
	        	// TODO: fiddle with coordinate manipulation
	            stars.push_back(
	            	std::make_unique<star>(
	            		coordinate(79, (layer % 2 == 0) ? 23 - sy : sy)
	            		, (layer == 1) ? '.' : '.'
	            		, layer));
	            gw.add_object(stars.back().get());
	    	}
    	} 
    }
}

void cleanup(game_world& gw, std::vector<world_object*>& kill_list, std::vector<std::unique_ptr<star>>& stars)
{
    for (auto* o : kill_list)
    {
        gw.remove_object(o);
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

void logic_loop(game_world& gw, engine_state& es)
{
    using namespace std::literals::chrono_literals;
    using namespace std::placeholders;
    auto next_autoscroll = system_clock::now() + 150ms;

    std::vector<std::unique_ptr<world_object>> dynamic_spawns;

    es.scroll_rate = 150ms;

    es.on_tick = std::bind(&game_world::on_tick, &gw, _1);
    es.on_scroll = std::bind(&game_world::on_scroll, &gw, _1);

    while (es.is_running)
    {
        std::unique_lock pause_lock(es.pause_mutex);
        es.pause_cv.wait(pause_lock, [&es](){ return !es.is_paused; });

        if (gw.progress == 20)
        {
            static bool spawn0 {true};

            if (spawn0)
            {
                dynamic_spawns.push_back(
                    std::make_unique<rhombus>(
                        coordinate { 40, 11}
                    )
                );
                gw.add_object(dynamic_spawns.back().get());
                spawn0 = false;
            }
        }
  /*      if (gw.progress == 40)
        {
            static bool spawn1 {true};

            if (spawn1)
            {
                enemies.push_back(
                    std::make_unique<torus>(
                        coordinate { 40, 20}
                    )
                );
                gw.add_object(enemies.back().get());
                spawn1 = false;
            }
        }
        if (gw.progress == 80)
        {
            static bool spawn2 {true};

            if (spawn2)
            {
                enemies.push_back(
                std::make_unique<rhombus>(
                    coordinate { 50, 5}
                    )
                );
                gw.add_object(enemies.back().get());
                spawn2 = false;
            }
        }
        if (gw.progress == 120)
        {
            static bool spawn3 {true};

            if (spawn3)
            {
                enemies.push_back(
                std::make_unique<diamond>(
                    coordinate { 60, 12}
                    )
                );
                gw.add_object(enemies.back().get());
                spawn3 = false;
            }
        }*/

        es.tick();
    }
}

int main()
{
    std::srand(std::time({}));

    player p { { 20, 10 } };

    game_world gw;
    gw.add_object(&p);

    render_autoscroll renderer{};
    terminal_screen term(gw, renderer);
    input_listener input(term.fd());

    input.add_callback('w', [&p](){
        p.set_move_intent(NORTH);
    });
    input.add_callback('a', [&p](){
        p.set_move_intent(WEST);
    });
    input.add_callback('s', [&p](){
        p.set_move_intent(SOUTH);
    });
    input.add_callback('d', [&p](){
        p.set_move_intent(EAST);
    });
    input.add_callback(' ', [&p](){ 
        p.set_move_intent(STATIC); 
    });
    input.add_callback('f', [&gw](){         
        gw.process_attacks(); 
    });
    
    
    engine_state es { input };

    std::thread render_thread { render_loop, std::ref(term), std::ref(es) };
    std::thread logic_thread { logic_loop, std::ref(gw), std::ref(es) };

    render_thread.join();
    logic_thread.join();
    
    return 0;
}
