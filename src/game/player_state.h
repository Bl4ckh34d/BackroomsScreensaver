#pragma once

// Gameplay-owned player state model. Renderer fields are migrated onto this
// struct in small slices so behavior can stay stable between builds.

struct PlayerSnapshotState {
    XMFLOAT3 position{};
    float yaw = 0.0f;
    float bodyYaw = 0.0f;
    float pitch = -0.055f;
    float stepPhase = 0.0f;
    float smoothedMoveSpeed = 0.0f;
    float runIntensity = 0.0f;
    float runEffort = 0.0f;
    float breathPhase = 0.0f;
    bool flashlightEnabled = true;
    float health = 100.0f;
    float stamina = 100.0f;
    float verticalOffset = 0.0f;
    float staminaRegenDelay = 0.0f;
    float audibleNoiseRadiusMeters = 0.0f;
    bool sprintStaminaLocked = false;
    bool tunnelCrouchLocked = false;
    float crouchBlend = 0.0f;
    float tunnelPostureHoldTimer = 0.0f;
    float tunnelLeanTarget = 0.0f;
    float tunnelLeanAmount = 0.0f;
    float tunnelLeanSideTarget = 1.0f;
    float tunnelLeanSide = 1.0f;
    bool interactPressedLastFrame = false;
    std::vector<uint16_t> visitedTiles;
};

struct PlayerState {
    XMFLOAT3 position{};
    float yaw = 0.0f;
    float bodyYaw = 0.0f;
    float pitch = -0.055f;

    float health = 100.0f;
    float stamina = 100.0f;
    float staminaRegenDelay = 0.0f;
    bool sprintStaminaLocked = false;

    float stepPhase = 0.0f;
    float smoothedMoveSpeed = 0.0f;
    float runIntensity = 0.0f;
    float runEffort = 0.0f;
    float breathPhase = 0.0f;

    float verticalOffset = 0.0f;
    float verticalVelocity = 0.0f;
    bool grounded = true;
    bool jumpRequested = false;

    bool crouchRequested = false;
    bool tunnelCrouchLocked = false;
    float crouchBlend = 0.0f;
    float tunnelPostureHoldTimer = 0.0f;
    float tunnelLeanTarget = 0.0f;
    float tunnelLeanAmount = 0.0f;
    float tunnelLeanSideTarget = 1.0f;
    float tunnelLeanSide = 1.0f;

    bool interactPressedLastFrame = false;
    bool flashlightEnabled = true;
    bool flashlightPressedLastFrame = false;

    float audibleNoiseRadiusMeters = 0.0f;
    std::vector<uint16_t> visitedTiles;

    void Kill();
    void RefillStamina();
    void RestoreVitals(float restoredHealth, float restoredStamina);
    void RestoreFullVitals();
    void ResetSessionState(const XMFLOAT3& startPosition, float initialSmoothedMoveSpeed, size_t visitedTileCount);
    void RestoreSavedRunState(
        const XMFLOAT3& savedPosition,
        float savedYaw,
        float savedBodyYaw,
        float savedPitch,
        float savedHealth,
        float savedStamina);
    PlayerSnapshotState CaptureSnapshotState() const;
    void RestoreSnapshotState(PlayerSnapshotState snapshot);
};
