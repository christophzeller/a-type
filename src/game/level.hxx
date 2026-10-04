#pragma once
#include "engine/objects.hxx"

#include <cstddef>
#include <map>
#include <vector>

struct spawn_info
{
    std::size_t type; // enum
    coordinate location;
    std::vector<coordinate> waypoints;
};

using spawn_map = std::map<std::size_t, std::vector<spawn_info>>;

struct level1
{
    level1()
    {
        std::vector<coordinate> wp;
        wp.push_back(coordinate {-5, 5} );
        spawn_info bob;
        bob.type = 0;
        bob.location = coordinate {75, 5};
        bob.waypoints = wp;

        wp.clear();
        wp.push_back(coordinate {10, 15} );
        wp.push_back(coordinate {10, 5} );
        wp.push_back(coordinate {70, 5} );
        wp.push_back(coordinate {70, 15} );
        spawn_info rob;
        rob.type = 0;
        rob.location = coordinate {75, 10};
        rob.waypoints = wp;

        std::vector<spawn_info> spawn_list;
        spawn_list.push_back(bob);
        spawn_list.push_back(rob);
        spawn_list.push_back(
            {
                0,
                coordinate {50, 20},
                { coordinate{60, -5} }
            }
        );

        spawns[30] = spawn_list;
    }

    spawn_map spawns;
};
