#pragma once

struct FixtureShadowRuntimeState {
    bool active = false;
    XMFLOAT3 position{0.0f, 0.0f, 0.0f};
    float range = 0.0f;
};
