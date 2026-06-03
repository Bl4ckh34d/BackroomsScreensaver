// Session reset and maze restart helpers.
// Included inside Renderer's private section from player_camera_movement.inl.

    void ResetSimulation() {
#include "player_reset_camera_path_state.inl"
#include "player_reset_effect_particles_lamps.inl"
#include "player_reset_scare_state.inl"
#include "player_reset_view_threat_state.inl"
#include "player_reset_world_content.inl"
#include "player_reset_debug_overrides.inl"
    }
