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

void GameWorld::RestoreFlashlightState(bool flashlightEnabled) {
    player.flashlightEnabled = flashlightEnabled;
    player.flashlightPressedLastFrame = false;
}

void GameWorld::RestoreInteractLatch(bool interactPressedLastFrame) {
    player.interactPressedLastFrame = interactPressedLastFrame;
}

void GameWorld::ResetPlayerInputLatches(bool flashlightEnabled) {
    PlayerController::ResetInputLatches(player, flashlightEnabled);
}

void GameWorld::RefillPlayerStamina() {
    player.RefillStamina();
}

void GameWorld::RestorePlayerFullVitals() {
    player.RestoreFullVitals();
}

void GameWorld::ResetPlayerForSession(float initialSmoothedMoveSpeed) {
    player.ResetSessionState(
        StartWorldCenter(1.45f),
        initialSmoothedMoveSpeed,
        static_cast<size_t>(std::max(0, maze.w * maze.h)));
}

void GameWorld::SetPlayerCameraPose(const XMFLOAT3& position, float yaw, float bodyYaw, float pitch) {
    player.position = position;
    player.yaw = yaw;
    player.bodyYaw = bodyYaw;
    player.pitch = pitch;
}

void GameWorld::SetPlayerHorizontalPosition(float x, float z) {
    player.position.x = x;
    player.position.z = z;
}

void GameWorld::AdvancePlayerStepPhase(float metersMoved, float speedMetersPerSecond, const PlayerMovementTuning& tuning) {
    PlayerController::AdvanceStepPhase(player, metersMoved, speedMetersPerSecond, tuning);
}

void GameWorld::MovePlayerHorizontalAndAdvanceStep(
    float x,
    float z,
    float metersMoved,
    float speedMetersPerSecond,
    const PlayerMovementTuning& tuning) {
    SetPlayerHorizontalPosition(x, z);
    AdvancePlayerStepPhase(metersMoved, speedMetersPerSecond, tuning);
}

PlayerManualControlResult GameWorld::UpdatePlayerManualControl(
    const PlayerManualControlInput& input,
    const PlayerManualControlServices& services) {
    return PlayerController::UpdateManualControl(player, input, services);
}
