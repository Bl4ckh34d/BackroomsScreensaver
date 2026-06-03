#include "../platform/platform_headers.h"
#include "../core/maze_types.h"
#include "../core/math_utils.h"
#include "../core/constants.h"
#include "../debug/effect_debug_constants.h"
#include "../config/settings.h"
#include "../audio/audio_engine.h"
#include "../audio/game_audio_events.h"
#include "../maze/maze.h"
#include "../gameplay/playable_progression_types.h"
#include "../monster/monster_state.h"

#include "player_controller.h"
#include "player_state.h"
#include "game_world.h"

bool GameWorld::BeginExitTransition() {
    if (exitTransitionActive) return false;
    exitTransitionActive = true;
    exitTransitionTimer = 0.0f;
    return true;
}

void GameWorld::AdvanceExitTransition(float dt) {
    if (!exitTransitionActive) return;
    exitTransitionTimer += dt;
}

void GameWorld::EndExitTransition() {
    exitTransitionActive = false;
}

bool GameWorld::BeginDeath() {
    if (deathActive) return false;
    deathActive = true;
    deathTimer = 0.0f;
    monster.killCount = std::min(monster.killCount + 1, 16);
    return true;
}

bool GameWorld::BeginPlayerDeath() {
    player.Kill();
    return BeginDeath();
}

void GameWorld::AdvanceDeath(float dt) {
    if (!deathActive) return;
    deathTimer += dt;
}
