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

void GameWorld::ResetSessionState() {
    collectiblePagesCollected = 0;
    savePoint = {};
    playerSoundPulses.clear();
    audioEvents.clear();
    exitTransitionActive = false;
    exitTransitionTimer = 0.0f;
    deathActive = false;
    deathTimer = 0.0f;
}

GameWorldRenderSnapshot GameWorld::BuildRenderSnapshot() const {
    GameWorldRenderSnapshot snapshot{};
    snapshot.maze = &maze;
    snapshot.playerPosition = player.position;
    snapshot.playerYaw = player.yaw;
    snapshot.playerBodyYaw = player.bodyYaw;
    snapshot.playerPitch = player.pitch;
    snapshot.playerHealth = player.health;
    snapshot.playerStamina = player.stamina;
    snapshot.playerFlashlightEnabled = player.flashlightEnabled;
    snapshot.playerAudibleNoiseRadiusMeters = player.audibleNoiseRadiusMeters;
    snapshot.playerStepPhase = player.stepPhase;
    snapshot.playerRunIntensity = player.runIntensity;
    snapshot.playerRunEffort = player.runEffort;
    snapshot.playerTunnelLeanSide = player.tunnelLeanSide;
    snapshot.playerTunnelLeanAmount = player.tunnelLeanAmount;
    snapshot.monsterPosition = monster.position;
    snapshot.monsterYaw = monster.yaw;
    snapshot.monsterKillCount = monster.killCount;
    snapshot.monsterChasePanic = monster.chasePanic;
    snapshot.monsterRoamBurstTimer = monster.roamBurstTimer;
    snapshot.monsterChasingVisible = monster.chasingVisible;
    snapshot.monsterHeardPlayerNow = monster.heardPlayerNow;
    snapshot.monsterCanSeePlayerNow = monster.canSeePlayerNow;
    snapshot.monsterHasSoundTarget = monster.hasSound;
    snapshot.monsterHasLastKnownTarget = monster.hasLastKnown;
    snapshot.monsterGoal = monster.goal;
    snapshot.monsterSoundTile = monster.soundTile;
    snapshot.monsterLastKnownTile = monster.lastKnownTile;
    snapshot.monsterRoamTile = monster.roamTile;
    snapshot.monsterPath = &monster.path;
    snapshot.monsterPathIndex = monster.pathIndex;
    snapshot.playerSoundPulses = &playerSoundPulses;
    snapshot.collectiblePagesCollected = collectiblePagesCollected;
    snapshot.collectiblePages = &collectiblePages;
    snapshot.savePoint = savePoint;
    snapshot.progressionEnabled = progressionEnabled;
    snapshot.exitTransitionActive = exitTransitionActive;
    snapshot.exitTransitionTimer = exitTransitionTimer;
    snapshot.deathActive = deathActive;
    snapshot.deathTimer = deathTimer;
    return snapshot;
}

GameWorldSnapshotState GameWorld::CaptureSnapshotState() const {
    GameWorldSnapshotState snapshot{};
    snapshot.maze = maze;
    snapshot.playerState = player.CaptureSnapshotState();
    snapshot.monsterState = monster.CaptureSnapshotState();
    snapshot.playableRun = playableRun;
    snapshot.collectiblePages = collectiblePages;
    snapshot.collectiblePagesCollected = collectiblePagesCollected;
    snapshot.savePoint = savePoint;
    snapshot.playerSoundPulses = playerSoundPulses;
    snapshot.progressionEnabled = progressionEnabled;
    snapshot.exitTransitionActive = exitTransitionActive;
    snapshot.exitTransitionTimer = exitTransitionTimer;
    snapshot.deathActive = deathActive;
    snapshot.deathTimer = deathTimer;
    return snapshot;
}

void GameWorld::RestoreSnapshotGenerationState(const GameWorldSnapshotState& snapshot) {
    playableRun = snapshot.playableRun;
    maze = snapshot.maze;
    progressionEnabled = snapshot.progressionEnabled;
}

void GameWorld::RestoreSnapshotRuntimeState(GameWorldSnapshotState snapshot) {
    player.RestoreSnapshotState(std::move(snapshot.playerState));
    monster.RestoreSnapshotState(std::move(snapshot.monsterState));
    playerSoundPulses = std::move(snapshot.playerSoundPulses);
    audioEvents.clear();
    exitTransitionActive = snapshot.exitTransitionActive;
    exitTransitionTimer = snapshot.exitTransitionTimer;
    deathActive = snapshot.deathActive;
    deathTimer = snapshot.deathTimer;
    collectiblePages = snapshot.collectiblePages;
    collectiblePagesCollected = snapshot.collectiblePagesCollected;
    savePoint = snapshot.savePoint;
}

void GameWorld::ResetMonsterKillCount() {
    monster.ResetKillCount();
}

void GameWorld::ResetPlayableRun() {
    playableRun.Reset();
}
