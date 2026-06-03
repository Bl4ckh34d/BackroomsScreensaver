#pragma once

struct EffectFloatRange {
    float min = 0.0f;
    float max = 0.0f;
};

struct EffectIntRange {
    int min = 0;
    int max = 0;
};

constexpr float kEffectBloodLoopSeconds = 56.0f;
constexpr float kEffectBloodFullSpreadAge = 48.0f;
constexpr float kEffectWaterLoopSeconds = 7.5f;
constexpr float kEffectAirVentLoopSeconds = 6.2f;
constexpr float kEffectBrokenLampLoopSeconds = 5.2f;
constexpr float kEffectStaticLoopSeconds = 8.0f;
constexpr EffectFloatRange kEffectBrokenLampSparkIntensity{2.2f, 4.1f};
constexpr float kEffectBrokenLampChainIntensityScale = 0.70f;
constexpr EffectIntRange kEffectBrokenLampChainBursts{1, 3};
constexpr EffectFloatRange kEffectAirVentSteamIntensity{1.9f, 3.2f};
constexpr int kEffectAirVentPanelDropEvery = 3;
constexpr float kEffectAirVentPanelDropChance = 1.0f / static_cast<float>(kEffectAirVentPanelDropEvery);
constexpr float kVentDropFloorY = 0.026f;
constexpr size_t kMaxVentDrops = 64;
constexpr float kEffectDebugAmbientLight = 0.32f;
constexpr float kEffectDebugExposureMax = 1.0f;
constexpr float kEffectDebugBloomMax = 0.30f;
constexpr float kEffectDebugFlashlightIntensity = 0.82f;
constexpr float kEffectDebugLampIntensity = 1.35f;
constexpr float kEffectBloodStreamCount = 30.0f;
constexpr float kEffectBloodStreamThickness = 0.88f;
constexpr float kEffectBloodShaderQuality = 1.0f;
constexpr float kEffectBloodRevealRadius = 100000.0f;
