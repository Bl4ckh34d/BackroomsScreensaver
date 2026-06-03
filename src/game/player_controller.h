#pragma once

#include "game_input.h"
#include "player_state.h"

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

struct PlayerMoveIntentInput {
    float moveX = 0.0f;
    float moveZ = 0.0f;
    float yaw = 0.0f;
};

struct PlayerMoveIntentResult {
    bool wantsMove = false;
    XMFLOAT3 moveDir{0.0f, 0.0f, 1.0f};
};

struct PlayerManualMoveInput {
    float dt = 0.0f;
    bool wantsMove = false;
    float currentSpeed = 0.0f;
    float targetSpeed = 0.0f;
};

struct PlayerManualMoveResult {
    float smoothedSpeed = 0.0f;
    float moveDistance = 0.0f;
    bool movedSafely = false;
};

struct PlayerCollisionMoveRequest {
    float stepX = 0.0f;
    float stepZ = 0.0f;
    float distance = 0.0f;
    float speed = 0.0f;
};

struct PlayerManualCollisionMoveInput {
    float dt = 0.0f;
    bool wantsMove = false;
    float targetSpeed = 0.0f;
    XMFLOAT3 moveDir{0.0f, 0.0f, 1.0f};
};

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

struct PlayerManualLookInput {
    float dt = 0.0f;
    float lookDeltaX = 0.0f;
    float lookDeltaY = 0.0f;
    float mouseSensitivity = 1.0f;
    bool invertMouseY = false;
    float yaw = 0.0f;
    float pitch = 0.0f;
    float yawDelta = 0.0f;
    float pitchDelta = 0.0f;
};

struct PlayerManualLookResult {
    float yaw = 0.0f;
    float bodyYaw = 0.0f;
    float pitch = 0.0f;
    float yawDelta = 0.0f;
    float pitchDelta = 0.0f;
};

struct PlayerFlashlightInputResult {
    bool toggled = false;
    bool enabled = true;
};

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

class PlayerController {
public:
    static PlayerMoveIntentResult BuildMoveIntent(const PlayerMoveIntentInput& input);
    static PlayerManualMoveResult UpdateManualMove(const PlayerManualMoveInput& input);

    template <typename MoveCallback>
    static PlayerManualMoveResult UpdateManualHorizontalMove(
        PlayerState& state,
        const PlayerManualCollisionMoveInput& input,
        MoveCallback moveCallback);

    static PlayerManualControlResult UpdateManualControl(
        PlayerState& state,
        const PlayerManualControlInput& input,
        const PlayerManualControlServices& services);

    static PlayerRunMotionResult UpdateRunMotion(const PlayerRunMotionInput& input);
    static void UpdateManualCameraHeight(PlayerState& state, const PlayerManualCameraHeightInput& input);
    static void AdvanceStepPhase(PlayerState& state, float metersMoved, float speedMetersPerSecond, const PlayerMovementTuning& tuning);
    static PlayerManualLookResult UpdateManualLook(const PlayerManualLookInput& input);
    static PlayerCrouchResult UpdateCrouch(PlayerState& state, const PlayerCrouchInput& input);
    static PlayerFlashlightInputResult UpdateFlashlightInput(PlayerState& state, bool inputEnabled, bool flashlightPressed);
    static bool ConsumeInteractPress(PlayerState& state, bool interactPressed);
    static void ResetInputLatches(PlayerState& state, bool flashlightEnabled);
    static PlayerStaminaResult UpdateStamina(PlayerState& state, const PlayerStaminaInput& input);
};

#include "player_controller.inl"
