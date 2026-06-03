#pragma once

#include "../config/settings.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <ostream>
enum class PlayableScareTier : uint8_t {
    None = 0,
    Harmless = 1,
    Water = 2,
    Blood = 3,
    Flesh = 4
};

static constexpr int kPlayableLevelsPerLayer = 5;

struct CustomGameSpec {
    static constexpr int kScareTypeCount = 5;

    int layer = 1;
    int mazeWidth = 15;
    int mazeHeight = 15;
    int roomCount = 3;
    bool brokenLampScares = true;
    bool airVentScares = true;
    bool waterScares = true;
    bool bloodWorldScares = true;
    bool fleshWorldScares = true;
    bool omukadeBoss = true;
    bool eightPages = true;
    int mapDirtPercent = 48;
    int paperDensityPercent = 100;
    int propDensityPercent = 100;
    int lampOnPercent = 100;
    int lampFlickerPercent = 10;
    int lampSparkPercent = 15;
    int fogStartMeters = 0;
    int fogEndMeters = 28;
    int fogDarknessPercent = 100;
    int jumpscareChancePercent = 15;
    int jumpscareStartMinSeconds = 0;
    int jumpscareStartMaxSeconds = 0;
    std::array<int, kScareTypeCount> scareChancePercent{{15, 15, 15, 15, 15}};
    std::array<int, kScareTypeCount> scareStartMinSeconds{{0, 0, 0, 0, 0}};
    std::array<int, kScareTypeCount> scareStartMaxSeconds{{0, 0, 0, 0, 0}};

    void Normalize() {
        layer = std::max(1, layer);
        mazeWidth = std::clamp(mazeWidth | 1, 3, 151);
        mazeHeight = std::clamp(mazeHeight | 1, 3, 151);
        roomCount = std::clamp(roomCount, 0, 80);
        mapDirtPercent = std::clamp(mapDirtPercent, 0, 100);
        paperDensityPercent = std::clamp(paperDensityPercent, 0, 400);
        propDensityPercent = std::clamp(propDensityPercent, 0, 400);
        lampOnPercent = std::clamp(lampOnPercent, 0, 100);
        lampFlickerPercent = std::clamp(lampFlickerPercent, 0, 100);
        lampSparkPercent = std::clamp(lampSparkPercent, 0, 100);
        fogStartMeters = std::clamp(fogStartMeters, 0, 200);
        fogEndMeters = std::clamp(fogEndMeters, fogStartMeters + 1, 300);
        fogDarknessPercent = std::clamp(fogDarknessPercent, 0, 100);
        jumpscareChancePercent = std::clamp(jumpscareChancePercent, 0, 100);
        jumpscareStartMinSeconds = std::clamp(jumpscareStartMinSeconds, 0, 600);
        jumpscareStartMaxSeconds = std::clamp(jumpscareStartMaxSeconds, jumpscareStartMinSeconds, 600);
        for (size_t i = 0; i < kScareTypeCount; ++i) {
            scareChancePercent[i] = std::clamp(scareChancePercent[i], 0, 100);
            scareStartMinSeconds[i] = std::clamp(scareStartMinSeconds[i], 0, 600);
            scareStartMaxSeconds[i] = std::clamp(scareStartMaxSeconds[i], scareStartMinSeconds[i], 600);
        }
    }

    PlayableScareTier ScareTier() const {
        if (fleshWorldScares) return PlayableScareTier::Flesh;
        if (bloodWorldScares) return PlayableScareTier::Blood;
        if (waterScares) return PlayableScareTier::Water;
        if (brokenLampScares || airVentScares) return PlayableScareTier::Harmless;
        return PlayableScareTier::None;
    }

    bool AnyScareEnabled() const {
        return brokenLampScares || airVentScares || waterScares || bloodWorldScares || fleshWorldScares;
    }

    int MaxScareChancePercent() const {
        int maxChance = 0;
        for (int chance : scareChancePercent) {
            maxChance = std::max(maxChance, std::clamp(chance, 0, 100));
        }
        return maxChance;
    }

    void ApplyEnvironmentSettings(Settings& target, const Settings& baseline) const {
        target.paperDensity = baseline.paperDensity * (static_cast<float>(paperDensityPercent) / 100.0f);
        target.hallwayPaperRunDensity = baseline.hallwayPaperRunDensity * (static_cast<float>(paperDensityPercent) / 100.0f);
        target.chairDensity = baseline.chairDensity * (static_cast<float>(propDensityPercent) / 100.0f);
        target.metalCabinetDensity = baseline.metalCabinetDensity * (static_cast<float>(propDensityPercent) / 100.0f);
        target.lampOnRatio = static_cast<float>(lampOnPercent) / 100.0f;
        target.lampFlickerRatio = static_cast<float>(lampFlickerPercent) / 100.0f;
        target.sparkEmitterRatio = static_cast<float>(lampSparkPercent) / 100.0f;
        target.fogStartMeters = static_cast<float>(fogStartMeters);
        target.fogEndMeters = std::max(target.fogStartMeters + 0.1f, static_cast<float>(fogEndMeters));
        target.fogDarkness = static_cast<float>(fogDarknessPercent) / 100.0f;
    }

    void ApplyScareSettings(Settings& target) const {
        target.brokenLampScaresEnabled = brokenLampScares;
        target.airVentScaresEnabled = airVentScares;
        target.waterDamageEnabled = waterScares;
        target.roomCount = std::clamp(roomCount, 0, 80);
        if (!waterScares) target.waterDamageDensity = 0.0f;
        target.bloodWorldFlicker = bloodWorldScares;
        if (!bloodWorldScares) target.bloodWorldCoverage = 0.0f;
        target.fleshFlicker = fleshWorldScares;
        target.jumpscareFrequency = AnyScareEnabled()
            ? Clamp01(static_cast<float>(MaxScareChancePercent()) / 100.0f)
            : 0.0f;
        if (!omukadeBoss) target.monsterIgnorePlayer = true;
    }

    void ApplyRuntimeSettings(Settings& target, const Settings& baseline) const {
        ApplyEnvironmentSettings(target, baseline);
        ApplyScareSettings(target);
    }

    void WriteStartNotice(std::wostream& out, int runtimeMazeWidth, int runtimeMazeHeight) const {
        out << L"Custom Game  " << runtimeMazeWidth << L"x" << runtimeMazeHeight;
        if (omukadeBoss) out << L"  Omukade active";
    }
};
