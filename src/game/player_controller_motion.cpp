#include "../platform/platform_headers.h"

#include "../core/math_utils.h"
#include "player_state.h"
#include "player_controller.h"
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
