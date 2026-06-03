Settings LoadSettings() {
    EnsureSettingsFile();
    Settings s;
#include "settings_load_window_maze.inl"
#include "settings_load_textures.inl"
#include "settings_load_lighting.inl"
#include "settings_load_camera_controls.inl"
#include "settings_load_audio.inl"
#include "settings_load_atmosphere.inl"
#include "settings_load_effect_tuning.inl"
#include "settings_load_dread.inl"
#include "settings_load_monster.inl"
    return s;
}
