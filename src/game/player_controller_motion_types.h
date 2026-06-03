#pragma once

struct PlayerRunMotionInput {
    float dt = 0.0f;
    float smoothedSpeed = 0.0f;
    float walkSpeed = 0.0f;
    float sprintSpeed = 0.0f;
    bool wantsSprint = false;
    bool staminaJog = false;
    float runIntensity = 0.0f;
    float runEffort = 0.0f;
    float breathPhase = 0.0f;
};

struct PlayerRunMotionResult {
    float moveBlend = 0.0f;
    float runBlend = 0.0f;
    float runIntensity = 0.0f;
    float runEffort = 0.0f;
    float breathPhase = 0.0f;
};

struct PlayerMovementTuning {
    float walkSpeed = 0.0f;
    float runSpeed = 0.0f;
};
