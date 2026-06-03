#pragma once

enum class GpuProfileMarker : size_t {
    FrameStart = 0,
    ClearTargets,
    DynamicGeometry,
    FlashlightShadow,
    FixtureShadow,
    Uploads,
    MainOpaque,
    FloorCeiling,
    DynamicOpaque,
    StaticWater,
    StaticTransparent,
    DynamicTransparent,
    PostProcess,
    Overlays,
    FrameEnd,
    Count
};

static constexpr size_t kGpuProfileMarkerCount = static_cast<size_t>(GpuProfileMarker::Count);
static constexpr size_t kGpuProfileFrameCount = 4;

struct GpuProfileFrame {
    ComPtr<ID3D11Query> disjoint;
    std::array<ComPtr<ID3D11Query>, kGpuProfileMarkerCount> timestamps;
    std::array<double, kGpuProfileMarkerCount> cpuMarkersMs{};
    bool issued = false;
    bool open = false;
    uint64_t frameId = 0;
};

struct GpuProfileRuntimeState {
    std::array<GpuProfileFrame, kGpuProfileFrameCount> frames;
    bool available = false;
    bool frameOpen = false;
    size_t writeIndex = 0;
    uint64_t frameCounter = 0;
};
