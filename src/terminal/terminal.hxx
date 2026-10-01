#pragma once

#include "../world/objects.hxx"
#include "../world/game_world.hxx"
#include "../world/engine_state.hxx"

#include <chrono>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include <fcntl.h>
#include <sys/select.h>
#include <termios.h>
#include <unistd.h>

using namespace std::chrono;
using namespace std::literals::chrono_literals;

struct terminal
{

    terminal()
    {
        tty_fd = open("/dev/tty", O_RDWR);
    }

    ~terminal()
    {
        close(tty_fd);
    }

    int tty_fd;
};


struct i_render_strategy
{
    virtual void draw_world(game_world& gw) = 0;
    virtual void draw_ui(engine_state es, game_world& gw) = 0;
};

/*struct render_static : public i_render_strategy
{
    void draw_world(game_world& gw) override;
    void draw_ui(game_world& gw) override;
};*/

struct render_autoscroll : public i_render_strategy
{
    void draw_world(game_world& gw) override;
    void draw_ui(engine_state es, game_world& gw) override;
};


struct terminal_screen 
{
    using buffer = std::vector<std::string>;
    using depth_info = std::vector<std::vector<std::size_t>>;
    struct render_buffers
    {
        buffer surface;
        depth_info depth;
    };

    explicit terminal_screen(i_render_strategy& renderer, int fd=0);
    virtual ~terminal_screen();

    void draw(engine_state es, game_world& gw) 
    {
        auto t0 = system_clock::now();
        renderer_.draw_world(gw); 
        auto t1 = system_clock::now();
        renderer_.draw_ui(es, gw); 
    };

    inline int fd() { return tty_fd; }

private:
    terminal_screen::buffer get_renderbuffer();
    i_render_strategy& renderer_;

    system_clock::duration render_time;

    struct termios original_terminal_settings;

    int tty_fd { -1 };
    std::size_t width  { 80 };
    std::size_t height { 24 };
    
};

terminal_screen::buffer terminal_screen::get_renderbuffer()
{
    buffer tmp;

    for (auto i = 0; i < height; ++i)
    {
        tmp.push_back(std::string(width, ' '));
    }

    return tmp;
}

terminal_screen::terminal_screen(i_render_strategy& renderer, int fd)
 : renderer_(renderer)
 , tty_fd(fd)
{
    std::cout << __PRETTY_FUNCTION__ << "\n";
   
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
}

terminal_screen::render_buffers get_renderbuffers(std::size_t width = 80, std::size_t height = 24)
{
    terminal_screen::render_buffers tmp;

    for (auto i = 0; i < height; ++i)
    {
        tmp.surface.push_back(std::string(width, ' '));
        tmp.depth.push_back( std::vector<std::size_t>(width, 0)  );
    }

    return tmp;
}


bool is_in_screenspace(const coordinate& c)
{
    return (c.x >= 0) && (c.y >= 0) && (c.x <= 79) && (c.y <= 23);
}

void render_sprite(terminal_screen::render_buffers& rb, const render_info& ri, coordinate sub_tile)
{
    coordinate chunk = ri.bb.top_left;
    chunk += sub_tile;
    if (is_in_screenspace(chunk))
    {
        auto symbol = ri.model[sub_tile.y * ri.bb.dimensions.x + sub_tile.x];
        if (symbol != ri.transparency && rb.depth[chunk.y][chunk.x] < ri.z_order)
        {
            rb.surface[chunk.y][chunk.x] = symbol;
            rb.depth[chunk.y][chunk.x] = ri.z_order;
        }
    }
}

void render_object(terminal_screen::render_buffers& rb, i_drawable* object)
{
    render_info ri = object->get_render_info();
    
    for (auto j = 0; j < ri.bb.dimensions.y; ++j)
    {
        for (auto i = 0; i < ri.bb.dimensions.x; ++i)
        {
            render_sprite(rb, ri, {i, j});
        }
    }
}

void render_autoscroll::draw_world(game_world& gw)
{
    auto buf = get_renderbuffers();

    std::lock_guard<std::mutex> world_guard(world_mutex);
    
    for (auto* obj : gw.drawables)
    {   
        render_object(buf, obj);
    }

    std::cout << "\033[H";
    std::string zipped { ' ' };
    zipped.reserve(80*24+24);

    for (const auto& line : buf.surface)
    {
        zipped += line;
        zipped += "\n";
    }
    std::cout << zipped;

}

void render_autoscroll::draw_ui(engine_state es, game_world& gw)
{
    auto width = 80;
    std::string mt { "" };
    std::string paused { "PAUSED" };
    std::string instructions { "WASD to move, SPACEBAR to stop moving, P to pause/unpause, Q to quit" };
    std::string distance { "DISTANCE: "};
    std::string debug { "debug: " };
    
    mt.resize(width);
    paused.resize(width);
    instructions.resize(width);
    distance += std::to_string(gw.progress);

    std::cout << (false ? paused : mt) << "\n" << instructions << "\n" << distance << "\n";
}
