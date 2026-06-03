#pragma once

#include "../core/constants.h"

#include <array>
#include <cstdint>
#include <string>

// Settings data model.
struct Settings {
#include "settings_window_maze_fields.inl"
#include "settings_texture_fields.inl"
#include "settings_lighting_fields.inl"
#include "settings_camera_fields.inl"
#include "settings_input_audio_fields.inl"
#include "settings_atmosphere_effect_fields.inl"
#include "settings_dread_monster_fields.inl"
};