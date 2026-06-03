#include "../platform/platform_headers.h"

#include "../core/math_utils.h"
#include "player_state.h"
#include "player_controller.h"
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
