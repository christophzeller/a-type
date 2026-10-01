#include "enemies.hxx"

using namespace std::chrono;
using namespace std::literals::chrono_literals;


struct rhombus : public enemy//, world_object, i_movable, i_drawable
{
    rhombus(coordinate position) : enemy(position)
    {
        model_chars = { 
        '/', '=', '\\',
        '\\', '=', '/'
        };
	    model_dimensions = { 3, 2 };

    	move_timer = 80ms;
    	move_speed = 80ms;

    	current_waypoint = 0;
        waypoints.push_back( coordinate { -5, 15} );

    	guns.clear();
    	guns.push_back({-4, 0});
    	guns.push_back({-4, 1});
    } //world_object('<', position, [](){}) {}

    render_info get_render_info() override 
    {
        bounding_box bb;
        bb.top_left = position_;
        bb.dimensions = model_dimensions;

        return render_info{ bb, model_chars, 'X', 40 };
    }

    void update(system_clock::duration delta_t) override
    {
        enemy::update(delta_t);
        navigate(*this, waypoints[current_waypoint]);

        if (position_ == waypoints[current_waypoint])
        {
            current_waypoint++;
            current_waypoint %= waypoints.size();
        }
    }

    bool allow_oob() const override { return true; }
};
