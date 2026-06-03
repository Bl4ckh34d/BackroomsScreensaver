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
