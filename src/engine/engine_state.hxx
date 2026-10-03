#pragma once
#include "engine/terminal/types.hxx"

struct engine_state
{
    resolution screen_dimensions;
    bool is_paused { false };
    bool is_running { true };
    // frame counters, times and stuff?
};
