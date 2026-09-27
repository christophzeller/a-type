#pragma once

#include "../world/enemies.hxx"

#include "../world/player.hxx"
#include "../world/world.hxx"

#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <format>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <fcntl.h>
#include <sys/select.h>
#include <termios.h>
#include <unistd.h>

struct i_render_strategy
{
    virtual void draw_world(game_world& gw) = 0;
    virtual void draw_ui(game_world& gw) = 0;
};

/*struct render_static : public i_render_strategy
{
    void draw_world(game_world& gw) override;
    void draw_ui(game_world& gw) override;
};*/

struct render_autoscroll : public i_render_strategy
{
    void draw_world(game_world& gw) override;
    void draw_ui(game_world& gw) override;
};


struct terminal_screen 
{
    using buffer = std::vector<std::string>;

    explicit terminal_screen(game_world& gw, i_render_strategy& renderer);
    virtual ~terminal_screen();

    void draw() { renderer_.draw_world(gw_); renderer_.draw_ui(gw_); };

    inline int fd() { return tty_fd; }

private:
    terminal_screen::buffer get_renderbuffer();
    i_render_strategy& renderer_;

    game_world& gw_;
    struct termios original_terminal_settings;

    int tty_fd { -1 };
    std::size_t width  { 80 };
    std::size_t height { 24 };
    
};

terminal_screen::buffer get_renderbuffer(std::size_t width = 80, std::size_t height = 24)
{
    terminal_screen::buffer tmp;

    for (auto i = 0; i < height; ++i)
    {
        tmp.push_back(std::string(width, ' '));
    }

    return tmp;

}

terminal_screen::buffer terminal_screen::get_renderbuffer()
{
    buffer tmp;

    for (auto i = 0; i < height; ++i)
    {
        tmp.push_back(std::string(width, ' '));
    }

    return tmp;
}

terminal_screen::terminal_screen(game_world& gw, i_render_strategy& renderer)
 : gw_(gw)
 , renderer_(renderer)
{
    std::cout << __PRETTY_FUNCTION__ << "\n";

    tty_fd = open("/dev/tty", O_RDWR);
    std::cout << "tty_fd: " << tty_fd << "\n";
    
    if (tcgetattr(tty_fd, &original_terminal_settings))
    {
        perror("get");
        std::cout << "error getattr\n";
    }
    struct termios settings = original_terminal_settings;
    settings.c_lflag &= ~(ICANON | ECHO);

    if (tcsetattr(tty_fd, TCSANOW, &settings))
    {
        perror("set");
        std::cout << "error setattr\n";
    }

    std::cout << "\e[?25l";
}

terminal_screen::~terminal_screen()
{
    tcsetattr(tty_fd, TCSAFLUSH, &original_terminal_settings);
    std::cout << "\e[?25h";
    close(tty_fd);
}

bool is_in_screenspace(coordinate c)
{
    return (c.x >= 0) && (c.y >= 0) && (c.x <= 79) && (c.y <= 23);
}

void render_object(terminal_screen::buffer& buf, i_drawable* object)
{
    render_info ri = object->get_render_info();
    for (auto j = 0; j < ri.bb.dimensions.y; ++j)
    {
        for (auto i = 0; i < ri.bb.dimensions.x; ++i)
        {
            coordinate chunk = ri.bb.top_left;
            chunk += {i, j};
            if (is_in_screenspace(chunk))
            {
                auto symbol = ri.model[j * ri.bb.dimensions.x + i];
                if (symbol != ri.transparency)
                    buf[chunk.y][chunk.x] = symbol;
            }
        }
    }
}


void render_autoscroll::draw_world(game_world& gw)
{
    auto buf = get_renderbuffer();

    std::lock_guard<std::mutex> world_guard(world_mutex);
    torus* t;
    
    for (auto* obj : gw.everything)
    {   
        // skip for now, render last
        if (obj == gw.the_player)
            continue;

        if (auto t_ = dynamic_cast<torus*>(obj))
            t = t_;
        // drawable->where().y, x
        if (auto* drawable = dynamic_cast<i_drawable*>(obj))
        {
            render_object(buf, drawable);
            //auto ss = obj->position_;
            //ss += { -gw.progress, 0};
            //if (is_in_screenspace(ss))
            //    buf[ss.y][ss.x] = drawable->get_representation();
        }
    }

    render_object(buf, dynamic_cast<i_drawable*>(t));

    render_object(buf, dynamic_cast<i_drawable*>(gw.the_player));
    
    std::cout << "\033[H";
    for (const auto& line : buf)
        std::cout << line << "\n";

}

void render_autoscroll::draw_ui(game_world& gw)
{
    auto width = 80;
    std::string mt { "" };
    std::string paused { "PAUSED" };
    std::string instructions { "WASD to move, SPACEBAR to stop moving, P to pause/unpause, Q to quit" };
    std::string distance { "DISTANCE: "};
    
    mt.resize(width);
    paused.resize(width);
    instructions.resize(width);
    distance += std::to_string(gw.progress);

    std::cout << (false ? paused : mt) << "\n" << instructions << "\n" << distance << "\n";
}






//struct render_scroller : public i_render_strategy
// move render loop to strategy
//
    // read full level description from game_world
    // get scrolling progression / offset
    // get framebuffer
    // read chunk: framebuffer dimensions @ world + offset
    // store world, only replace new column/row?
    // draw

    // in logic thread: advance scrolling
// starfield:
// 
