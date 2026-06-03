#pragma once

struct MenuPlaquePlacement {
    XMFLOAT3 center{};
    XMFLOAT3 right{1.0f, 0.0f, 0.0f};
    XMFLOAT3 inward{0.0f, 0.0f, -1.0f};
    float halfW = 0.72f;
    float halfH = 0.122f;
};
