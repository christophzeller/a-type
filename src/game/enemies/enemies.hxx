#pragma once

#include "engine/ai/ai.hxx"
#include "engine/objects.hxx"
#include "engine/utilities.hxx"

#include <chrono>
#include <vector>

using namespace std::chrono;
using namespace std::literals::chrono_literals;
using namespace engine;
using namespace engine::ai;
using namespace engine::utilities;

// TODO: 
// AI movement: navigate to a set of waypoints. possible presets:
// current player position once, then continue off-screen (aimed non-homing collision course)
// current player position continuous (collision course)
// current player position for a set amount of time (time-bound homing)
// diagonal across the screen
// z-pattern across the screen
// circle around a point
// avoid player line of fire
// hold position relative to an object (formation)
// hold position relative to two objects (block)
// 

struct enemy : public world_object, i_movable, i_drawable, i_collider, i_attacker, i_shootable
{
    enemy(coordinate position, 
        system_clock::duration attack_interval = 1250ms, 
        system_clock::duration move_interval = 250ms, 
        std::vector<coordinate> flight_plan = {}
//        std::vector<char> model = { '?' }
        )
    : world_object(position)
    , attack_timer(attack_interval)
    , attack_frequency(attack_interval)
    , move_timer(move_interval)
    , move_speed(move_interval)
    , waypoints(flight_plan)
    {
    }


    enemy(coordinate position, 
        system_clock::duration move_interval, 
        system_clock::duration attack_interval)
    : enemy(position, move_interval, attack_interval, waypoints)
    {
    }

    
    void update(system_clock::duration delta_t) override 
    {
        if (move_intent != STATIC)
        {
            move_timer -= delta_t;
        }
 
        attack_timer -= delta_t;
        if (attack_timer < 0ms)
        {
            attack_intent = true;
            attack_timer = attack_frequency;
        }
    }

    render_info get_render_info() override { return render_info(); }

    void set_move_intent(direction dir) override 
    {
    	move_intent = dir; 
	    // todo: sync time
    }
    
    direction get_move_intent(bool reset = true) override 
    { 
        if (reset) 
        { 
            auto tmp = move_intent; 
            move_intent = STATIC; 
            return tmp;
        } 
    	else 
        	return move_intent; 
	}

    coordinate get_position() override { return position_; }

    bool move() override
    {
        if (move_timer < 0ms)
        {	
            move_timer = move_speed;
            return true;
        }
        return false;
    }

    bool allow_oob() const override { return false; }

    bounding_box get_bounding_box() const override
    {
        bounding_box bb;
        bb.top_left = coordinate{position_};
        bb.dimensions = coordinate{model_dimensions};

        return bb;  // TODO: member & update on move?
    }

    extents get_extents() const
    {
        return engine::utilities::get_extents(get_bounding_box());
    }
    
    void on_collision(i_collider* other) override 
    {
    }

    bool attack() override
    {
        if (attack_intent)
        {
            attack_intent = false;
            return true;
        }
        else
        {
            return false;
        }
    }

    const attacks get_attacks() const override
    {
        attacks atks;
        for (const auto& gun : guns)
        {
            attack_info atk;
            atk.spawn = position_ + gun.spawn;
            atk.dir = gun.dir;
            atks.push_back( atk );
        }

        return atks;
    }

    void deal_damage(int dmg) override
    {
        hp -= 1;
    }

    bool is_alive() const override
    {
        return hp > 0;
    }

    bool boom() const override
    {
        return true;
    }

    system_clock::duration attack_timer { 1s };
    system_clock::duration attack_frequency { 3s };

    system_clock::duration move_timer { 100ms };
    system_clock::duration move_speed { 100ms };
    direction move_intent { STATIC };

    bool attack_intent { false };
    std::vector<gun> guns { { WEST, {-4, 0}} };
        
    std::vector<coordinate> waypoints {};
    std::size_t current_waypoint { 0 };

    std::vector<char> model_chars { '?' };
    coordinate model_dimensions { 1, 1 };

    int hp { 1 };
};
