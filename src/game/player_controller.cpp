#include "../platform/platform_headers.h"

#include "../core/math_utils.h"
#include "player_state.h"
#include "player_controller.h"

PlayerMoveIntentResult PlayerController::BuildMoveIntent(const PlayerMoveIntentInput& input) {
    float inputX = std::clamp(input.moveX, -1.0f, 1.0f);
    float inputZ = std::clamp(input.moveZ, -1.0f, 1.0f);
    float inputLen = std::sqrt(inputX * inputX + inputZ * inputZ);
    if (inputLen > 1.0f) {
        inputX /= inputLen;
        inputZ /= inputLen;
        inputLen = 1.0f;
    }

    PlayerMoveIntentResult result{};
    result.wantsMove = inputLen > 0.001f;
    const XMFLOAT3 forward{std::sin(input.yaw), 0.0f, std::cos(input.yaw)};
    const XMFLOAT3 right{std::cos(input.yaw), 0.0f, -std::sin(input.yaw)};
    XMFLOAT3 moveDir{
        right.x * inputX + forward.x * inputZ,
        0.0f,
        right.z * inputX + forward.z * inputZ
    };
    const float lenSq = moveDir.x * moveDir.x + moveDir.z * moveDir.z;
    if (lenSq > 0.000001f) {
        const float invLen = 1.0f / std::sqrt(lenSq);
        moveDir.x *= invLen;
        moveDir.z *= invLen;
    } else {
        moveDir = forward;
    }
    result.moveDir = moveDir;
    return result;
}

PlayerManualMoveResult PlayerController::UpdateManualMove(const PlayerManualMoveInput& input) {
    PlayerManualMoveResult result{};
    const float speedAccel = input.wantsMove
        ? (input.targetSpeed > input.currentSpeed ? 4.9f : 6.2f)
        : 7.0f;
    result.smoothedSpeed = MoveTowards(input.currentSpeed, input.wantsMove ? input.targetSpeed : 0.0f,
        speedAccel * input.dt);
    result.moveDistance = result.smoothedSpeed * input.dt;
    return result;
}

PlayerManualControlResult PlayerController::UpdateManualControl(
    PlayerState& state,
    const PlayerManualControlInput& input,
    const PlayerManualControlServices& services) {
    PlayerManualControlResult result{};

    PlayerManualLookInput lookInput{};
    lookInput.dt = input.dt;
    lookInput.lookDeltaX = input.input.lookDeltaX;
    lookInput.lookDeltaY = input.input.lookDeltaY;
    lookInput.mouseSensitivity = input.mouseSensitivity;
    lookInput.invertMouseY = input.invertMouseY;
    lookInput.yaw = state.yaw;
    lookInput.pitch = state.pitch;
    lookInput.yawDelta = input.manualLookYawDelta;
    lookInput.pitchDelta = input.manualLookPitchDelta;
    PlayerManualLookResult lookResult = UpdateManualLook(lookInput);
    state.yaw = lookResult.yaw;
    state.bodyYaw = lookResult.bodyYaw;
    state.pitch = lookResult.pitch;
    result.manualLookYawDelta = lookResult.yawDelta;
    result.manualLookPitchDelta = lookResult.pitchDelta;

    PlayerMoveIntentInput moveIntentInput{};
    moveIntentInput.moveX = input.input.moveX;
    moveIntentInput.moveZ = input.input.moveZ;
    moveIntentInput.yaw = state.yaw;
    PlayerMoveIntentResult moveIntent = BuildMoveIntent(moveIntentInput);
    result.wantsMove = moveIntent.wantsMove;
    result.moveDir = moveIntent.moveDir;

    PlayerManualPostureRequest postureRequest{};
    postureRequest.moveDir = result.moveDir;
    postureRequest.wantsMove = result.wantsMove;
    PlayerManualPostureResult postureResult = services.updatePosture
        ? services.updatePosture(services.context, postureRequest)
        : PlayerManualPostureResult{};
    const bool crouching = postureResult.crouching;
    PlayerCrouchInput crouchInput{};
    crouchInput.dt = input.dt;
    crouchInput.crouchActive = crouching;
    PlayerCrouchResult crouch = UpdateCrouch(state, crouchInput);
    if (services.ensureFootprint) services.ensureFootprint(services.context);

    PlayerStaminaInput staminaInput{};
    staminaInput.dt = input.dt;
    staminaInput.sprintPressed = input.input.sprint;
    staminaInput.wantsMove = result.wantsMove;
    staminaInput.crouching = crouching;
    staminaInput.crouchSpeedScale = crouch.speedScale;
    staminaInput.walkSpeed = input.movement.walkSpeed;
    staminaInput.runSpeed = input.movement.runSpeed;
    staminaInput.infiniteStamina = input.infiniteStamina;
    PlayerStaminaResult staminaResult = UpdateStamina(state, staminaInput);

    PlayerManualCollisionMoveInput moveInput{};
    moveInput.dt = input.dt;
    moveInput.wantsMove = result.wantsMove;
    moveInput.targetSpeed = staminaResult.targetSpeed;
    moveInput.moveDir = result.moveDir;
    PlayerManualMoveResult moveResult = UpdateManualHorizontalMove(
        state,
        moveInput,
        [&](const PlayerCollisionMoveRequest& request) {
            return services.move ? services.move(services.context, request) : false;
        });
    result.moveDistance = moveResult.moveDistance;

    result.interactPressed = ConsumeInteractPress(state, input.input.interact);

    PlayerRunMotionInput runInput{};
    runInput.dt = input.dt;
    runInput.smoothedSpeed = state.smoothedMoveSpeed;
    runInput.walkSpeed = staminaResult.walkSpeed;
    runInput.sprintSpeed = staminaResult.sprintSpeed;
    runInput.wantsSprint = staminaResult.wantsSprint;
    runInput.staminaJog = staminaResult.staminaJog;
    runInput.runIntensity = state.runIntensity;
    runInput.runEffort = state.runEffort;
    runInput.breathPhase = state.breathPhase;
    PlayerRunMotionResult runResult = UpdateRunMotion(runInput);
    result.moveBlend = runResult.moveBlend;
    result.runBlend = runResult.runBlend;
    state.runIntensity = runResult.runIntensity;
    state.runEffort = runResult.runEffort;
    state.breathPhase = runResult.breathPhase;
    if (result.wantsMove) {
        AdvanceStepPhase(state, result.moveDistance, std::max(0.1f, state.smoothedMoveSpeed), input.movement);
    }

    PlayerManualCameraHeightInput heightInput{};
    heightInput.dt = input.dt;
    heightInput.headBobAmount = input.headBobAmount;
    heightInput.moveBlend = result.moveBlend;
    heightInput.runBlend = result.runBlend;
    heightInput.crouch = crouch;
    UpdateManualCameraHeight(state, heightInput);
    return result;
}

PlayerRunMotionResult PlayerController::UpdateRunMotion(const PlayerRunMotionInput& input) {
    PlayerRunMotionResult result{};
    result.moveBlend = Clamp01(input.smoothedSpeed / std::max(0.1f, input.sprintSpeed));
    result.runBlend = Clamp01((input.smoothedSpeed - input.walkSpeed) /
        std::max(0.1f, input.sprintSpeed - input.walkSpeed));
    const float jogBlend = input.staminaJog
        ? std::min(0.62f, std::max(0.40f, result.runBlend))
        : result.runBlend;
    const float sprintMotionTarget = input.wantsSprint
        ? (input.staminaJog ? jogBlend : result.runBlend)
        : result.moveBlend * 0.45f;
    const float sprintEffortTarget = input.wantsSprint
        ? (input.staminaJog ? 0.52f : result.runBlend)
        : 0.0f;

    result.runIntensity = input.runIntensity + (sprintMotionTarget - input.runIntensity) *
        std::min(1.0f, input.dt * (input.wantsSprint ? 3.2f : 2.6f));
    result.runEffort = input.runEffort + (sprintEffortTarget - input.runEffort) *
        std::min(1.0f, input.dt * (input.wantsSprint ? 2.4f : 3.0f));
    result.breathPhase = input.breathPhase +
        input.dt * (0.95f + result.runEffort * 2.2f + result.runIntensity * 0.85f) * kPi;
    if (result.breathPhase > kPi * 128.0f) {
        result.breathPhase = std::fmod(result.breathPhase, kPi * 2.0f);
    }
    return result;
}

void PlayerController::UpdateManualCameraHeight(PlayerState& state, const PlayerManualCameraHeightInput& input) {
    float configuredBob = std::min(input.headBobAmount, 0.10f);
    float bobAmount = configuredBob * Lerp(0.045f, 0.22f, Clamp01(input.moveBlend * 0.55f + input.runBlend * 0.45f)) *
        input.crouch.headBobScale;
    float stepWave = std::sin(state.stepPhase * 2.0f);
    float verticalBob = stepWave * bobAmount;
    float sideBob = stepWave * (0.00055f + state.runEffort * 0.0011f) * input.crouch.sideBobScale;
    float breathY = std::sin(state.breathPhase) *
        (0.002f + state.runIntensity * 0.0045f + state.runEffort * 0.008f);
    float desiredY = input.crouch.eyeHeight + state.verticalOffset + verticalBob + sideBob + breathY;
    state.position.y += (desiredY - state.position.y) * (1.0f - std::exp(-input.dt * input.crouch.eyeResponse));
}

void PlayerController::AdvanceStepPhase(
    PlayerState& state,
    float metersMoved,
    float speedMetersPerSecond,
    const PlayerMovementTuning& tuning) {
    if (metersMoved <= 0.0001f || speedMetersPerSecond <= 0.0001f) return;
    float runBlend = Clamp01((speedMetersPerSecond - tuning.walkSpeed) / std::max(0.1f, tuning.runSpeed - tuning.walkSpeed));
    float strideMeters = Lerp(1.42f, 1.58f, SmoothStep(0.0f, 1.0f, runBlend));
    state.stepPhase += metersMoved * (kPi / std::max(0.25f, strideMeters));
    if (state.stepPhase > kPi * 128.0f) {
        state.stepPhase = std::fmod(state.stepPhase, kPi * 2.0f);
    }
}

PlayerManualLookResult PlayerController::UpdateManualLook(const PlayerManualLookInput& input) {
    constexpr float kMouseYawScale = 0.0022f;
    constexpr float kMousePitchScale = 0.0018f;
    constexpr float kManualPitchLimit = 1.55334f;

    const float mouseScale = std::clamp(input.mouseSensitivity, 0.1f, 5.0f);
    const float pitchSign = input.invertMouseY ? 1.0f : -1.0f;
    const float rawYawDelta = input.lookDeltaX * kMouseYawScale * mouseScale;
    const float rawPitchDelta = input.lookDeltaY * kMousePitchScale * mouseScale * pitchSign;
    const float lookSmoothing = 1.0f - std::exp(-input.dt * 26.0f);

    PlayerManualLookResult result{};
    result.yawDelta = input.yawDelta + (rawYawDelta - input.yawDelta) * lookSmoothing;
    result.pitchDelta = input.pitchDelta + (rawPitchDelta - input.pitchDelta) *
        std::min(1.0f, input.dt * 18.0f);

    result.yaw = input.yaw + result.yawDelta;
    result.bodyYaw = result.yaw;
    float adjustedPitchDelta = result.pitchDelta;
    if (adjustedPitchDelta * input.pitch > 0.0f) {
        const float edge = SmoothStep(0.64f, 1.0f, std::abs(input.pitch) / kManualPitchLimit);
        adjustedPitchDelta *= 1.0f + (0.16f - 1.0f) * edge;
    }
    result.pitch = std::clamp(input.pitch + adjustedPitchDelta, -kManualPitchLimit, kManualPitchLimit);
    return result;
}

PlayerCrouchResult PlayerController::UpdateCrouch(PlayerState& state, const PlayerCrouchInput& input) {
    const float target = input.crouchActive ? 1.0f : 0.0f;
    const float response = target > state.crouchBlend ? 14.0f : 8.4f;
    const float alpha = 1.0f - std::exp(-std::max(0.0f, input.dt) * response);
    state.crouchBlend += (target - state.crouchBlend) * alpha;
    if (state.crouchBlend < 0.001f && target <= 0.0f) state.crouchBlend = 0.0f;
    if (state.crouchBlend > 0.999f && target >= 1.0f) state.crouchBlend = 1.0f;

    PlayerCrouchResult result{};
    result.pose = SmoothStep(0.0f, 1.0f, state.crouchBlend);
    result.speedScale = 1.0f + (0.52f - 1.0f) * result.pose;
    result.eyeHeight = 1.45f + (1.12f - 1.45f) * result.pose;
    result.eyeResponse = result.pose > 0.5f ? 15.0f : 9.6f;
    result.headBobScale = 1.0f + (0.10f - 1.0f) * result.pose;
    result.sideBobScale = 1.0f - result.pose;
    return result;
}

PlayerFlashlightInputResult PlayerController::UpdateFlashlightInput(
    PlayerState& state,
    bool inputEnabled,
    bool flashlightPressed) {
    PlayerFlashlightInputResult result{};
    if (!inputEnabled) {
        state.flashlightPressedLastFrame = false;
        result.enabled = state.flashlightEnabled;
        return result;
    }

    if (flashlightPressed && !state.flashlightPressedLastFrame) {
        state.flashlightEnabled = !state.flashlightEnabled;
        result.toggled = true;
    }
    state.flashlightPressedLastFrame = flashlightPressed;
    result.enabled = state.flashlightEnabled;
    return result;
}

bool PlayerController::ConsumeInteractPress(PlayerState& state, bool interactPressed) {
    bool pressedNow = interactPressed && !state.interactPressedLastFrame;
    state.interactPressedLastFrame = interactPressed;
    return pressedNow;
}

void PlayerController::ResetInputLatches(PlayerState& state, bool flashlightEnabled) {
    state.flashlightEnabled = flashlightEnabled;
    state.flashlightPressedLastFrame = false;
    state.interactPressedLastFrame = false;
}

PlayerStaminaResult PlayerController::UpdateStamina(PlayerState& state, const PlayerStaminaInput& input) {
    PlayerStaminaResult result{};
    result.walkSpeed = input.walkSpeed;
    result.sprintSpeed = std::max(input.runSpeed, input.walkSpeed * 1.35f);

    if (input.infiniteStamina) {
        state.stamina = 100.0f;
        state.staminaRegenDelay = 0.0f;
        state.sprintStaminaLocked = false;
    }

    constexpr float kJogStamina = 100.0f / 3.0f;
    constexpr float kExhaustedStamina = 5.0f;
    if (state.stamina <= 0.001f) state.sprintStaminaLocked = true;
    if (state.sprintStaminaLocked && state.stamina >= kExhaustedStamina) state.sprintStaminaLocked = false;

    const bool sprintAllowed = input.infiniteStamina ||
        (!state.sprintStaminaLocked && state.stamina > 0.001f);
    result.wantsSprint = input.sprintPressed && input.wantsMove && !input.crouching && sprintAllowed;
    result.staminaJog = result.wantsSprint && !input.infiniteStamina && state.stamina <= kJogStamina;

    const float jogSpeed = input.walkSpeed + (result.sprintSpeed - input.walkSpeed) * 0.55f;
    result.targetSpeed = result.wantsSprint
        ? (result.staminaJog ? jogSpeed : result.sprintSpeed)
        : input.walkSpeed;
    result.targetSpeed *= input.crouchSpeedScale;

    if (result.wantsSprint && !input.infiniteStamina) {
        const float drainRate = state.stamina <= kExhaustedStamina
            ? (kExhaustedStamina / 30.0f)
            : (result.staminaJog ? 2.00f : 5.00f);
        state.stamina = std::max(0.0f, state.stamina - drainRate * input.dt);
        state.staminaRegenDelay = 0.85f;
        if (state.stamina <= 0.001f) {
            result.wantsSprint = false;
            state.sprintStaminaLocked = true;
            result.targetSpeed = input.walkSpeed;
        }
    } else if (result.wantsSprint && input.infiniteStamina) {
        state.stamina = 100.0f;
        state.staminaRegenDelay = 0.0f;
    } else {
        if (state.stamina < kExhaustedStamina) {
            state.staminaRegenDelay = 0.0f;
            state.stamina = std::min(100.0f, state.stamina + (kExhaustedStamina / 3.0f) * input.dt);
        } else {
            state.staminaRegenDelay = std::max(0.0f, state.staminaRegenDelay - input.dt);
            if (state.staminaRegenDelay <= 0.0f) {
                const float regenRate = state.stamina < kJogStamina ? 10.50f : 5.20f;
                state.stamina = std::min(100.0f, state.stamina + regenRate * input.dt);
            }
        }
    }

    return result;
}
