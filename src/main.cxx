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

void logic_loop(game_world& gw, engine_state& es)
{
    using namespace std::literals::chrono_literals;
    using namespace std::placeholders;
    auto next_autoscroll = system_clock::now() + 150ms;

    std::vector<std::unique_ptr<world_object>> dynamic_spawns;

    es.scroll_rate = 100ms;

    es.on_tick = std::bind(&game_world::on_tick, &gw, _1);
    es.on_scroll = std::bind(&game_world::on_scroll, &gw, _1);

    while (es.is_running)
    {
        std::unique_lock pause_lock(es.pause_mutex);
        es.pause_cv.wait(pause_lock, [&es](){ return !es.is_paused; });

        es.tick();
    }
}

int main()
{
    std::srand(std::time({}));

    player p { { 20, 10 } };

    game_world gw;
    gw.dynamic_spawns.reserve(128);
    gw.stars.reserve(384);
    gw.add_object(&p);

    terminal t {};
    auto fd = t.tty_fd;
    input_listener input(fd);
    engine_state es { input };    

    render_autoscroll renderer{};
    terminal_screen term(es, gw, renderer, fd);


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
    input.add_callback('f', [&p](){
        p.attack_intent = true;
    });
    input.add_callback('r', [&gw](){
        gw.spawn(0); 
    });
    input.add_callback('t', [&gw](){
        gw.spawn(1); 
    });
    input.add_callback('z', [&gw](){
        gw.spawn(2); 
    });
    input.add_callback('e', [&gw](){
        gw.spawn(-1); 
    });

    std::thread render_thread { render_loop, std::ref(term), std::ref(es) };
    std::thread logic_thread { logic_loop, std::ref(gw), std::ref(es) };

    render_thread.join();
    logic_thread.join();
    
    return 0;
}
