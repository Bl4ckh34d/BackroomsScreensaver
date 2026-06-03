#pragma once

struct StaticIndexChunk {
    UINT startIndex = 0;
    UINT indexCount = 0;
    XMFLOAT3 center{};
    float radius = 0.0f;
    int minTileX = 0;
    int minTileY = 0;
    int maxTileX = 0;
    int maxTileY = 0;
};

struct StaticInstanceData {
    XMFLOAT4 axisXOriginX{};
    XMFLOAT4 axisYOriginY{};
    XMFLOAT4 axisZOriginZ{};
    XMFLOAT4 materialOverrideVariant{-1.0f, 0.0f, 0.0f, 0.0f};
};

struct StaticInstanceChunk {
    UINT startIndex = 0;
    UINT indexCount = 0;
    INT baseVertex = 0;
    UINT startInstance = 0;
    UINT instanceCount = 0;
    XMFLOAT3 center{};
    float radius = 0.0f;
    int minTileX = 0;
    int minTileY = 0;
    int maxTileX = 0;
    int maxTileY = 0;
};
