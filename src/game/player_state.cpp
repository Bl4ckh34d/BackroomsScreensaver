#include "../platform/platform_headers.h"

#include "player_state.h"

void PlayerState::Kill() {
    health = 0.0f;
}

void PlayerState::RefillStamina() {
    stamina = 100.0f;
}

void PlayerState::RestoreVitals(float restoredHealth, float restoredStamina) {
    health = std::clamp(restoredHealth, 1.0f, 100.0f);
    stamina = std::clamp(restoredStamina, 0.0f, 100.0f);
}

void PlayerState::RestoreFullVitals() {
    health = 100.0f;
    RefillStamina();
}

void PlayerState::ResetSessionState(const XMFLOAT3& startPosition,
                                    float initialSmoothedMoveSpeed,
                                    size_t visitedTileCount) {
    position = startPosition;
    yaw = 0.0f;
    bodyYaw = 0.0f;
    pitch = -0.055f;

    RestoreFullVitals();
    staminaRegenDelay = 0.0f;
    sprintStaminaLocked = false;

    stepPhase = 0.0f;
    smoothedMoveSpeed = initialSmoothedMoveSpeed;
    runIntensity = 0.0f;
    runEffort = 0.0f;
    breathPhase = 0.0f;

    verticalOffset = 0.0f;
    verticalVelocity = 0.0f;
    grounded = true;
    jumpRequested = false;

    crouchRequested = false;
    tunnelCrouchLocked = false;
    crouchBlend = 0.0f;
    tunnelPostureHoldTimer = 0.0f;
    tunnelLeanTarget = 0.0f;
    tunnelLeanAmount = 0.0f;
    tunnelLeanSideTarget = 1.0f;
    tunnelLeanSide = 1.0f;

    interactPressedLastFrame = false;
    audibleNoiseRadiusMeters = 0.0f;
    visitedTiles.assign(visitedTileCount, 0);
}

void PlayerState::RestoreSavedRunState(const XMFLOAT3& savedPosition,
                                       float savedYaw,
                                       float savedBodyYaw,
                                       float savedPitch,
                                       float savedHealth,
                                       float savedStamina) {
    position = savedPosition;
    yaw = savedYaw;
    bodyYaw = savedBodyYaw;
    pitch = savedPitch;
    RestoreVitals(savedHealth, savedStamina);
}

PlayerSnapshotState PlayerState::CaptureSnapshotState() const {
    PlayerSnapshotState snapshot{};
    snapshot.position = position;
    snapshot.yaw = yaw;
    snapshot.bodyYaw = bodyYaw;
    snapshot.pitch = pitch;
    snapshot.stepPhase = stepPhase;
    snapshot.smoothedMoveSpeed = smoothedMoveSpeed;
    snapshot.runIntensity = runIntensity;
    snapshot.runEffort = runEffort;
    snapshot.breathPhase = breathPhase;
    snapshot.flashlightEnabled = flashlightEnabled;
    snapshot.health = health;
    snapshot.stamina = stamina;
    snapshot.verticalOffset = verticalOffset;
    snapshot.staminaRegenDelay = staminaRegenDelay;
    snapshot.audibleNoiseRadiusMeters = audibleNoiseRadiusMeters;
    snapshot.sprintStaminaLocked = sprintStaminaLocked;
    snapshot.tunnelCrouchLocked = tunnelCrouchLocked;
    snapshot.crouchBlend = crouchBlend;
    snapshot.tunnelPostureHoldTimer = tunnelPostureHoldTimer;
    snapshot.tunnelLeanTarget = tunnelLeanTarget;
    snapshot.tunnelLeanAmount = tunnelLeanAmount;
    snapshot.tunnelLeanSideTarget = tunnelLeanSideTarget;
    snapshot.tunnelLeanSide = tunnelLeanSide;
    snapshot.interactPressedLastFrame = interactPressedLastFrame;
    snapshot.visitedTiles = visitedTiles;
    return snapshot;
}

void PlayerState::RestoreSnapshotState(PlayerSnapshotState snapshot) {
    position = snapshot.position;
    yaw = snapshot.yaw;
    bodyYaw = snapshot.bodyYaw;
    pitch = snapshot.pitch;
    stepPhase = snapshot.stepPhase;
    smoothedMoveSpeed = snapshot.smoothedMoveSpeed;
    runIntensity = snapshot.runIntensity;
    runEffort = snapshot.runEffort;
    breathPhase = snapshot.breathPhase;
    flashlightEnabled = snapshot.flashlightEnabled;
    health = snapshot.health;
    stamina = snapshot.stamina;
    verticalOffset = snapshot.verticalOffset;
    staminaRegenDelay = snapshot.staminaRegenDelay;
    audibleNoiseRadiusMeters = snapshot.audibleNoiseRadiusMeters;
    sprintStaminaLocked = snapshot.sprintStaminaLocked;
    tunnelCrouchLocked = snapshot.tunnelCrouchLocked;
    crouchBlend = snapshot.crouchBlend;
    tunnelPostureHoldTimer = snapshot.tunnelPostureHoldTimer;
    tunnelLeanTarget = snapshot.tunnelLeanTarget;
    tunnelLeanAmount = snapshot.tunnelLeanAmount;
    tunnelLeanSideTarget = snapshot.tunnelLeanSideTarget;
    tunnelLeanSide = snapshot.tunnelLeanSide;
    interactPressedLastFrame = snapshot.interactPressedLastFrame;
    visitedTiles = std::move(snapshot.visitedTiles);
}
