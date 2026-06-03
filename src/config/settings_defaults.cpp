#include "../platform/platform_headers.h"

#include "settings.h"

std::wstring DefaultConfigText() {
    std::wostringstream s;
#include "settings_defaults_maze_window.inl"
#include "settings_defaults_textures_lighting.inl"
#include "settings_defaults_camera_controls.inl"
#include "settings_defaults_key_bindings.inl"
#include "settings_defaults_audio_atmosphere.inl"
#include "settings_defaults_effects_dread.inl"
#include "settings_defaults_debug_monster.inl"
    return s.str();
}
