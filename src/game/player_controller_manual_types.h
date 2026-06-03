#pragma once

#include "game_input.h"
#include "player_controller_move_types.h"
#include "player_controller_motion_types.h"
#include "player_controller_posture_types.h"
struct PlayerManualControlInput {
    float dt = 0.0f;
    GameInputSnapshot input{};
    float mouseSensitivity = 1.0f;
    bool invertMouseY = false;
    bool infiniteStamina = false;
    float headBobAmount = 0.0f;
    PlayerMovementTuning movement{};
    float manualLookYawDelta = 0.0f;
    float manualLookPitchDelta = 0.0f;
};

struct PlayerManualControlResult {
    bool wantsMove = false;
    XMFLOAT3 moveDir{0.0f, 0.0f, 1.0f};
    bool interactPressed = false;
    float manualLookYawDelta = 0.0f;
    float manualLookPitchDelta = 0.0f;
    float moveDistance = 0.0f;
    float moveBlend = 0.0f;
    float runBlend = 0.0f;
};

struct PlayerManualPostureRequest {
    XMFLOAT3 moveDir{0.0f, 0.0f, 1.0f};
    bool wantsMove = false;
};

struct PlayerManualPostureResult {
    bool crouching = false;
};

struct PlayerManualControlServices {
    void* context = nullptr;
    PlayerManualPostureResult (*updatePosture)(void* context, const PlayerManualPostureRequest& request) = nullptr;
    void (*ensureFootprint)(void* context) = nullptr;
    bool (*move)(void* context, const PlayerCollisionMoveRequest& request) = nullptr;
};
