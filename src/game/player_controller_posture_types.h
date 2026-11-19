#pragma once

inline constexpr float kPlayerStandingEyeHeight = 1.45f;
inline constexpr float kPlayerCrouchedEyeHeight = 1.12f;
inline constexpr float kPlayerCrouchedSpeedScale = 0.52f;

struct PlayerCrouchInput {
    float dt = 0.0f;
    bool crouchActive = false;
};

struct PlayerCrouchResult {
    float pose = 0.0f;
    float speedScale = 1.0f;
    float eyeHeight = 1.45f;
    float eyeResponse = 9.6f;
    float headBobScale = 1.0f;
    float sideBobScale = 1.0f;
};

struct PlayerManualCameraHeightInput {
    float dt = 0.0f;
    float headBobAmount = 0.0f;
    float moveBlend = 0.0f;
    float runBlend = 0.0f;
    PlayerCrouchResult crouch{};
};
