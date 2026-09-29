#pragma once

#include "objects.hxx"
#include <cstdlib>
#include <vector>

struct star : public world_object, i_drawable, i_movable
{
    star() = delete;
    explicit star(coordinate position, char symbol='*', std::size_t scroll_throttle=1) 
     : world_object(symbol, position, [](){})
     , throttle(scroll_throttle)
    {}

    void update(std::chrono::system_clock::duration delta_t) override {}
     
    ~star() = default;    
    star(const star&) = default;
    star(star&&) = default;
    star& operator=(star&) = default;
    star& operator=(star&&) = default;
    
    char get_representation() override 
    {
        if ((draw_count % blink_frequency) != 0)
        {
            ++draw_count;
            return symbol_;
        }

        if (blink_duration > 0)
        {
            --blink_duration;
            return ' ';
        }

        draw_count = 1;
        blink_duration = 4;
        return ' ';
        
    }

    render_info get_render_info() override 
    {
        bounding_box bb;
        bb.top_left = position_;
        bb.dimensions = {1, 1};
        model[0] = get_representation();
        return render_info{ bb, model };
    }
    
    void set_move_intent(direction dir) override {}
    direction get_move_intent(bool reset=false) override { return STATIC; }

    bool allow_autoscroll() override 
    { 
        if (scroll_count++ % throttle)
            return false;
        else
            return true;
    }
    bool move() override { return true; }
    coordinate get_position() override { return position_; }
    std::chrono::system_clock::duration get_speed() override { using namespace std::literals::chrono_literals; return 0ms; } // todo:: throttle * scroll rate

    bool allow_oob() override { return false; }

    std::vector<char> model { '*' };

    std::size_t blink_frequency { 1013 };
    std::size_t draw_count { std::rand() % blink_frequency };
    std::size_t blink_duration { 4 };

    std::size_t throttle { 1 };
    std::size_t scroll_count { 0 };
};

std::map<std::size_t, std::vector<int>> star_loop = 
{
{0, {10, 12, 20}},
{2, {0, 4}},
{3, {2}},
{8, {8}},
{10, {18}},
{11, {14}},
{12, {16}},
{14, {5, 22}},
{15, {0}},
{16, {9}},
{17, {7}},
{18, {15}},
{19, {11, 19}},
{20, {3, 6}},
{22, {21}},
{24, {4, 12, 23}},
{25, {1, 20}},
{28, {17}},
{29, {7}},
{33, {14}},
{34, {11}},
{35, {3}},
{37, {0, 8}},
{41, {6, 16}},
{42, {12}},
{44, {9}},
{45, {20, 23}},
{46, {1, 18}},
{47, {13}},
{48, {5}},
{51, {17}},
{52, {2, 8}},
{53, {4, 15}},
{54, {10, 19, 21}},
{57, {3, 14}},
{59, {11}},
{60, {0}},
{63, {20}},
{65, {22}},
{66, {0}},
{67, {5}},
{68, {17}},
{70, {8}},
{76, {0}},
{78, {23}},
{79, {6, 16}}
};



struct asteroid : public world_object, i_movable, i_drawable
{
    asteroid(coordinate position)
    : world_object('O', position, []() {})
    , animation_index(rand() % 12)
    , next_move(std::chrono::system_clock::now()) 
    {
        /*auto x = rand() % 3;

        switch (x)
        {
        case 0:
            drift_dir = NORTH;
        break;
        case 1:
            drift_dir = SOUTH;
        break;
        case 2:
            drift_dir = STATIC;
        break;
        }*/
    }

    // i_movable
    void set_move_intent(direction dir) override { move_intent = dir; }
    direction get_move_intent(bool reset = true) 
    { 
        using namespace std::literals::chrono_literals;
        
        if (std::chrono::system_clock::now() < next_move)
            return STATIC;

        next_move = std::chrono::system_clock::now() + milliseconds(move_interval);    
        return drift_dir;
    }

    bool allow_autoscroll() override { return true; }
    coordinate get_position() override { return position_; };

    bool move() override { return true; }
    std::chrono::system_clock::duration get_speed() override { using namespace std::literals::chrono_literals; return 0ms; } // todo:: throttle * scroll rate

    bool allow_oob() override { return false; }

    // i_drawable
    char get_representation() override 
    {
        return animation[animation_index++ % 8]; 
    }
 
    render_info get_render_info() override 
    {
        bounding_box bb;
        bb.top_left = position_;
        bb.dimensions = {1, 1};
        model[0] = get_representation();
        return render_info{ bb, model };
    }

    std::vector<char> model { '*' };

    std::chrono::time_point<std::chrono::system_clock> next_move;
    int move_interval { 1000 };
    char animation[12] { 'o', 'o', 'o', 'o', 'o', 'o', 'O', 'O', 'O', 'O', 'O', 'O'};
    std::size_t animation_index { 0 };
    direction move_intent { STATIC };

    direction drift_dir { STATIC };
};
