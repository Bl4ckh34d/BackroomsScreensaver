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
    const float dt = std::max(0.0f, input.dt);
    const float yawInputRate = std::abs(input.lookDeltaX) * kMouseYawScale / std::max(0.001f, dt);
    const float flick = SmoothStep(4.5f, 18.0f, yawInputRate);
    const float boostedYaw = rawYawDelta * Lerp(1.0f, 2.60f, flick);
    const float flickTailInput = boostedYaw * (0.18f * flick);

    // Apply most of a flick immediately, then ease its remaining angle to rest.
    constexpr float kResponseSeconds = 0.018f;
    constexpr float kFlickTailSeconds = 0.064f;
    const float response = -std::expm1(-dt / kResponseSeconds);
    const float tailResponse = -std::expm1(-dt / kFlickTailSeconds);
    const float pendingYaw = input.yawDelta + boostedYaw - flickTailInput;
    const float pendingFlickTail = input.yawFlickTail + flickTailInput;
    const float pendingPitch = input.pitchDelta + rawPitchDelta;
    const float appliedYaw = pendingYaw * response;
    const float appliedFlickTail = pendingFlickTail * tailResponse;
    const float appliedPitch = pendingPitch * response;

    PlayerManualLookResult result{};
    result.yawDelta = pendingYaw - appliedYaw;
    result.pitchDelta = pendingPitch - appliedPitch;
    result.yawFlickTail = pendingFlickTail - appliedFlickTail;

    result.yaw = input.yaw + appliedYaw + appliedFlickTail;
    constexpr float kTorsoResponseSeconds = 0.060f;
    const float torsoFollow = -std::expm1(-dt / kTorsoResponseSeconds);
    result.bodyYaw = input.bodyYaw + AngleWrap(result.yaw - input.bodyYaw) * torsoFollow;
    float adjustedPitchDelta = appliedPitch;
    if (adjustedPitchDelta * input.pitch > 0.0f) {
        const float edge = SmoothStep(0.64f, 1.0f, std::abs(input.pitch) / kManualPitchLimit);
        adjustedPitchDelta *= 1.0f + (0.16f - 1.0f) * edge;
    }
    result.pitch = std::clamp(input.pitch + adjustedPitchDelta, -kManualPitchLimit, kManualPitchLimit);
    if (std::abs(result.pitch) >= kManualPitchLimit && result.pitchDelta * result.pitch > 0.0f) {
        result.pitchDelta = 0.0f;
    }
    return result;
}
