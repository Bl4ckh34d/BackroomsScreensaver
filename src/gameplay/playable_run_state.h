#pragma once

#include "playable_progression_results.h"
#include "../core/constants.h"
#include "../core/math_utils.h"
#include "../core/maze_types.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <ostream>
#include <random>
#include <vector>
struct PlayableRunState {
    static constexpr int kLevelsPerLayer = kPlayableLevelsPerLayer;
    #include "playable_run_state_core.inl"
    #include "playable_run_state_save_plan.inl"
    #include "playable_run_state_saved_fields.inl"
    #include "playable_run_state_level_building.inl"
    #include "playable_run_state_begin_restore.inl"
    #include "playable_run_state_pages.inl"
    #include "playable_run_state_completion.inl"
};
