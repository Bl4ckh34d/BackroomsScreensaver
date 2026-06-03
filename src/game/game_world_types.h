#pragma once

struct CollectiblePage {
    XMFLOAT3 center{};
    XMFLOAT3 right{1.0f, 0.0f, 0.0f};
    XMFLOAT3 up{0.0f, 1.0f, 0.0f};
    XMFLOAT3 normal{0.0f, 0.0f, 1.0f};
    int pageIndex = -1;
    bool collected = false;
};

struct SavePoint {
    XMFLOAT3 pos{};
    float yaw = 0.0f;
    bool active = false;
};
