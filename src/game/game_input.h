#pragma once

struct GameInputSnapshot {
    float moveX = 0.0f;
    float moveZ = 0.0f;
    float lookDeltaX = 0.0f;
    float lookDeltaY = 0.0f;
    bool sprint = false;
    bool crouch = false;
    bool interact = false;
    bool flashlight = false;
    bool pause = false;
};
