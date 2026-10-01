#pragma once

#include "engine_state.hxx"
#include "../input/input.hxx"
#include "../terminal/terminal.hxx"
#include "game_world.hxx"
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

struct engine
{
    void render_loop()
    {
        using namespace std::literals::chrono_literals;

        time_point<system_clock> next_frame = system_clock::now();
        system_clock::duration frame_time { 33ms };

        std::map<std::size_t, std::vector<world_object*>> activation_list;
        
        while (is_running)
        {
            engine_state es;
            es.is_paused = is_paused;
            es.is_running = is_running;
            next_frame = system_clock::now() + frame_time;
            screen->draw(es, gw);
        }
    }

    void logic_loop()
    {
        using namespace std::literals::chrono_literals;
        using namespace std::placeholders;
        auto next_frame = system_clock::now() + 150ms;
        system_clock::duration frame_time { 33ms };

        scroll_rate = 100ms;

        while (is_running)
        {
            std::unique_lock pause_lock(pause_mutex);
            pause_cv.wait(pause_lock, [this](){ return !is_paused; });

            next_frame = system_clock::now() + frame_time;
            tick();
            auto throttle = next_frame - system_clock::now();
            if (throttle > 0ms)
                std::this_thread::sleep_for(5ms);
        }
    }

	void tick()
	{
		++current_tick;
		
		last_tick = now;
		now = system_clock::now();

		delta_t = now - last_tick;

		gw.on_tick(delta_t);
		if (now >= next_tick)
		{
			next_tick = now + scroll_rate;
			gw.on_scroll(delta_t);
		}
	}

	void run()
	{
        std::thread render_thread { &engine::render_loop, this };
        std::thread logic_thread { &engine::logic_loop, this };

        render_thread.join();
        logic_thread.join();
	}

    engine() : gw()
    {
        term = std::make_unique<terminal>();
        auto fd = term->tty_fd;

        renderer = std::make_unique<render_autoscroll>();

        screen = std::make_unique<terminal_screen>(*renderer, fd);
        
        input = std::make_unique<input_listener>(fd);


        input->add_callback('q', [this](){ is_paused = false; pause_cv.notify_all(); is_running = false; });
        input->add_callback('p', [this](){ is_paused = !is_paused; if (!is_paused) pause_cv.notify_all(); } ) ;

        input->add_callback('w', [this](){
            if (gw.the_player)
                gw.the_player->set_move_intent(NORTH);
        });
        input->add_callback('a', [this](){
            if (gw.the_player)
                gw.the_player->set_move_intent(WEST);
        });
        input->add_callback('s', [this](){
            if (gw.the_player)
                gw.the_player->set_move_intent(SOUTH);
        });
        input->add_callback('d', [this](){
            if (gw.the_player)
                gw.the_player->set_move_intent(EAST);
        });
        input->add_callback(' ', [this](){ 
            if (gw.the_player)
                gw.the_player->set_move_intent(STATIC); 
        });
        input->add_callback('f', [this](){
            if (gw.the_player)
                gw.the_player->attack_intent = true;
        });
        input->add_callback('r', [this](){
            gw.spawn(0); 
        });
        input->add_callback('t', [this](){
            gw.spawn(1); 
        });
        input->add_callback('z', [this](){
            gw.spawn(2); 
        });
        input->add_callback('e', [this](){
            gw.spawn(-1); 
        });
        input->add_callback('*', [this](){
            gw.spawn(42); 
        });

        gw.dynamic_spawns.reserve(128);
        gw.stars.reserve(384);
    }

    std::unique_ptr<terminal> term;
    std::unique_ptr<terminal_screen> screen;
    std::unique_ptr<input_listener> input;
    std::unique_ptr<i_render_strategy> renderer;
    game_world gw;

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
