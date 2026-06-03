// Config dialog Settings reconstruction.

Settings SettingsFromConfigControls(const ConfigState* state) {
    Settings s;
#include "config_dialog_settings_window_maze.inl"
#include "config_dialog_settings_textures.inl"
#include "config_dialog_settings_lighting.inl"
#include "config_dialog_settings_camera_audio.inl"
#include "config_dialog_settings_atmosphere.inl"
#include "config_dialog_settings_effect_tuning.inl"
#include "config_dialog_settings_dread.inl"
#include "config_dialog_settings_monster.inl"
    return s;
}
