#pragma once

#include "playable_custom_game_spec.h"

#include <algorithm>
struct PlayableLevelSpec {
    int layer = 1;
    int levelInLayer = 1;
    int mazeWidth = 15;
    int mazeHeight = 15;
    bool bossEncounter = false;
    float bossEncounterChance = 0.0f;
    PlayableScareTier scareTier = PlayableScareTier::None;

    bool NeedsLampFlickerRoll() const {
        return scareTier != PlayableScareTier::None;
    }

    void ApplyRuntimeSettings(
        Settings& target,
        const Settings& baseline,
        float lampFlickerUnit,
        bool darkLayerOneActive) const {
        const int scareTierValue = static_cast<int>(scareTier);
        target = baseline;
        target.mazeWidth = mazeWidth;
        target.mazeHeight = mazeHeight;
        target.roomCount = levelInLayer <= 2 ? 0 : std::min(3, levelInLayer - 2);

        const float loosePaperScaleByLevel[] = {0.0f, 0.055f, 0.24f, 0.58f, 1.0f};
        const float hallwayPaperScaleByLevel[] = {0.0f, 0.035f, 0.18f, 0.50f, 1.0f};
        const float clutterScaleByLevel[] = {0.0f, 0.035f, 0.18f, 0.48f, 1.0f};
        int levelIndex = std::clamp(levelInLayer, 1, kPlayableLevelsPerLayer) - 1;
        target.paperDensity *= loosePaperScaleByLevel[levelIndex];
        target.hallwayPaperRunDensity *= hallwayPaperScaleByLevel[levelIndex];
        target.chairDensity *= clutterScaleByLevel[levelIndex];

        target.lampOnRatio = 1.0f;
        target.brokenZoneRatio = scareTier == PlayableScareTier::None ? 0.0f : 0.05f;
        target.lampFlickerRatio = scareTier == PlayableScareTier::None ? 0.0f :
            Lerp(0.04f, 0.10f, lampFlickerUnit);
        target.sparkParticles = scareTier != PlayableScareTier::None && target.sparkParticles;

        if (darkLayerOneActive) {
            target.lampOnRatio = 0.0f;
            target.lampFlickerRatio = 0.0f;
            target.ambientLight = std::min(target.ambientLight, 0.002f);
        }

        constexpr float fogEndByLevel[kPlayableLevelsPerLayer] = {18.0f, 17.0f, 16.0f, 15.0f, 14.0f};
        constexpr float fogStrengthByLevel[kPlayableLevelsPerLayer] = {0.75f, 0.80f, 0.85f, 0.90f, 1.0f};
        target.fogStartMeters = 0.0f;
        target.fogEndMeters = fogEndByLevel[levelIndex];
        target.fogDarkness = fogStrengthByLevel[levelIndex];

        target.jumpscareFrequency = scareTier == PlayableScareTier::None ? 0.0f :
            std::max(target.jumpscareFrequency, 0.12f);
        target.waterDamageEnabled = scareTierValue >= static_cast<int>(PlayableScareTier::Water);
        target.waterDamageDensity = scareTierValue >= static_cast<int>(PlayableScareTier::Water) ? std::max(target.waterDamageDensity, 0.70f) : 0.0f;
        target.bloodSplatterDensity = scareTierValue >= static_cast<int>(PlayableScareTier::Blood) ? std::max(target.bloodSplatterDensity, 0.70f) : 0.0f;
        target.bloodWorldFlicker = scareTierValue >= static_cast<int>(PlayableScareTier::Blood);
        target.bloodWorldCoverage = scareTierValue >= static_cast<int>(PlayableScareTier::Blood) ? std::max(target.bloodWorldCoverage, 0.75f) : 0.0f;
        target.fleshFlicker = scareTierValue >= static_cast<int>(PlayableScareTier::Flesh);
        target.fleshAlwaysOn = false;
        target.monsterIgnorePlayer = !bossEncounter;
        if (bossEncounter) {
            target.monsterSpeed = std::max(target.monsterSpeed, levelInLayer >= 5 ? 0.78f : 0.62f);
            target.monsterSprintSpeed = std::max(target.monsterSprintSpeed, levelInLayer >= 5 ? 1.02f : 0.86f);
            target.monsterVisibleDistance = std::max(target.monsterVisibleDistance, levelInLayer >= 5 ? 15.0f : 10.0f);
        }
    }
};

inline PlayableLevelSpec CustomPlayableLevelSpec(const CustomGameSpec& spec) {
    PlayableLevelSpec level{};
    level.layer = spec.layer;
    level.levelInLayer = 1;
    level.mazeWidth = spec.mazeWidth;
    level.mazeHeight = spec.mazeHeight;
    level.bossEncounter = spec.omukadeBoss;
    level.bossEncounterChance = spec.omukadeBoss ? 1.0f : 0.0f;
    level.scareTier = spec.ScareTier();
    return level;
}
