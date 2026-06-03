#include "../platform/platform_headers.h"

#include "../core/math_utils.h"
#include "player_state.h"
#include "player_controller.h"
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
