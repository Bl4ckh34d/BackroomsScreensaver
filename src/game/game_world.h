#pragma once

#include "game_world_maze_generation.h"
#include "game_world_results.h"
#include "game_world_saved_restore.h"
#include "game_world_snapshot.h"

// Gameplay-owned world state. Renderer aliases still point into this aggregate
// while simulation systems move across in small, verifiable slices.
struct GameWorld {
#include "game_world_state_fields.inl"
#include "game_world_session_api.inl"
#include "game_world_saved_restore_api.inl"
#include "game_world_player_api.inl"
#include "game_world_monster_api.inl"
#include "game_world_playable_api.inl"
#include "game_world_playable_completion_api.inl"
#include "game_world_collectible_api.inl"
#include "game_world_audio_api.inl"
#include "game_world_lifecycle_api.inl"
};