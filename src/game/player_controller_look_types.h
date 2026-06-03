#pragma once

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
