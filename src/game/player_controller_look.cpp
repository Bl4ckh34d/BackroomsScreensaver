#include "../platform/platform_headers.h"

#include "../core/math_utils.h"
#include "player_state.h"
#include "player_controller.h"
PlayerManualLookResult PlayerController::UpdateManualLook(const PlayerManualLookInput& input) {
    constexpr float kMouseYawScale = 0.0022f;
    constexpr float kMousePitchScale = 0.0018f;
    constexpr float kManualPitchLimit = 1.55334f;

    const float mouseScale = std::clamp(input.mouseSensitivity, 0.1f, 5.0f);
    const float pitchSign = input.invertMouseY ? 1.0f : -1.0f;
    const float rawYawDelta = input.lookDeltaX * kMouseYawScale * mouseScale;
    const float rawPitchDelta = input.lookDeltaY * kMousePitchScale * mouseScale * pitchSign;

    PlayerManualLookResult result{};
    result.yawDelta = rawYawDelta;
    result.pitchDelta = rawPitchDelta;

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
