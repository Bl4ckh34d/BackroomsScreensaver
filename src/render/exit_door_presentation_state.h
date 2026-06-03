#pragma once

struct ExitDoorPresentationState {
    XMFLOAT3 center{};
    XMFLOAT3 normal{0.0f, 0.0f, 1.0f};
    XMFLOAT3 right{1.0f, 0.0f, 0.0f};
    XMFLOAT3 hinge{};
    XMFLOAT3 signLightPosition{};
    float signLightStrength = 0.0f;
    float angle = 0.0f;
};
