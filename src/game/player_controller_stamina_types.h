#pragma once

struct PlayerStaminaInput {
    float dt = 0.0f;
    bool sprintPressed = false;
    bool wantsMove = false;
    bool crouching = false;
    float crouchSpeedScale = 1.0f;
    float walkSpeed = 0.0f;
    float runSpeed = 0.0f;
    bool infiniteStamina = false;
};

struct PlayerStaminaResult {
    bool wantsSprint = false;
    bool staminaJog = false;
    float walkSpeed = 0.0f;
    float sprintSpeed = 0.0f;
    float targetSpeed = 0.0f;
};
