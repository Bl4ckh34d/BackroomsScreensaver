#pragma once

#include "../core/constants.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <vector>

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

struct PlayableLevelResult {
    int layer = 1;
    int levelInLayer = 1;
    float levelSeconds = 0.0f;
    float runSeconds = 0.0f;
    int score = 0;
    bool bossEncounter = false;
};

struct PlayableLevelCompletionUpdate {
    PlayableLevelResult result{};
    bool finalRun = false;
    int levelSecretTotal = 0;
    int levelSecretsFound = 0;
};

struct PlayableCustomScareGate {
    bool allowed = true;
    bool requiresRoll = false;
    float chance = 1.0f;
};

struct PlayableSavePointSpawnPlan {
    bool eligible = false;
    bool mustSpawn = false;
};

using SavedRunKeyValues = std::unordered_map<std::wstring, std::wstring>;

inline SavedRunKeyValues ParseSavedRunKeyValues(const std::wstring& text) {
    SavedRunKeyValues values;
    std::wistringstream in(text);
    std::wstring line;
    while (std::getline(in, line)) {
        if (!line.empty() && line.back() == L'\r') line.pop_back();
        size_t eq = line.find(L'=');
        if (eq == std::wstring::npos) continue;
        values[line.substr(0, eq)] = line.substr(eq + 1);
    }
    return values;
}

inline int SavedRunInt(const SavedRunKeyValues& values, const wchar_t* key, int fallback) {
    auto it = values.find(key);
    if (it == values.end()) return fallback;
    wchar_t* end = nullptr;
    long parsed = std::wcstol(it->second.c_str(), &end, 10);
    return end != it->second.c_str() ? static_cast<int>(parsed) : fallback;
}

inline float SavedRunFloat(const SavedRunKeyValues& values, const wchar_t* key, float fallback) {
    auto it = values.find(key);
    if (it == values.end()) return fallback;
    wchar_t* end = nullptr;
    float parsed = std::wcstof(it->second.c_str(), &end);
    return end != it->second.c_str() ? parsed : fallback;
}

inline std::wstring SavedRunString(const SavedRunKeyValues& values, const wchar_t* key) {
    auto it = values.find(key);
    return it == values.end() ? std::wstring{} : it->second;
}

struct PlayableRunState {
    static constexpr int kLevelsPerLayer = kPlayableLevelsPerLayer;

    struct SavedRunRestoreState {
        int layer = 1;
        int levelInLayer = 1;
        int completedLevels = 0;
        bool darkLayerOne = false;
        bool customGame = false;
        CustomGameSpec customSpec{};
        float customScareStartDelaySeconds = 0.0f;
        std::array<float, CustomGameSpec::kScareTypeCount> customScareStartDelayByTypeSeconds{{0.0f, 0.0f, 0.0f, 0.0f, 0.0f}};
        int saveItemTarget = 1;
        int saveItemsSpawned = 0;
        int layerPagesCollected = 0;
        std::array<int, kLevelsPerLayer> levelPageTargets{{2, 2, 2, 1, 1}};
        std::array<uint8_t, kCollectiblePageMaterialCount> layerPageCollected{};
        float runSeconds = 0.0f;
        float levelSeconds = 0.0f;
        int totalScore = 0;
        PlayableLevelSpec currentLevel{};
    };

    bool active = false;
    bool levelRunning = false;
    bool runFinished = false;
    bool scoreScreenActive = false;
    bool scoreScreenFinal = false;
    bool customGame = false;
    int layer = 1;
    int levelInLayer = 1;
    int completedLevels = 0;
    bool darkLayerOne = false;
    int saveItemTarget = 1;
    int saveItemsSpawned = 0;
    int layerPagesCollected = 0;
    std::array<int, kLevelsPerLayer> levelPageTargets{{2, 2, 2, 1, 1}};
    std::array<uint8_t, kCollectiblePageMaterialCount> layerPageCollected{};
    float runSeconds = 0.0f;
    float levelSeconds = 0.0f;
    float customScareStartDelaySeconds = 0.0f;
    std::array<float, CustomGameSpec::kScareTypeCount> customScareStartDelayByTypeSeconds{{0.0f, 0.0f, 0.0f, 0.0f, 0.0f}};
    int totalScore = 0;
    PlayableLevelSpec currentLevel{};
    CustomGameSpec customSpec{};
    PlayableLevelResult lastResult{};
    std::vector<PlayableLevelResult> completed;

    void Reset() {
        *this = {};
    }

    bool CanSaveActiveRun() const {
        return active && levelRunning;
    }

    bool IsActive() const {
        return active;
    }

    bool IsCustomGame() const {
        return customGame;
    }

    const CustomGameSpec& CustomSpec() const {
        return customSpec;
    }

    const PlayableLevelSpec& CurrentLevel() const {
        return currentLevel;
    }

    bool DarkLayerOneAppliesTo(int targetLayer) const {
        return active && darkLayerOne && targetLayer == 1;
    }

    bool CurrentLevelHasBossEncounter() const {
        return currentLevel.bossEncounter;
    }

    float CurrentLevelSeconds() const {
        return levelSeconds;
    }

    float RunSeconds() const {
        return runSeconds;
    }

    int TotalScore() const {
        return totalScore;
    }

    PlayableSavePointSpawnPlan BuildSavePointSpawnPlan() const {
        PlayableSavePointSpawnPlan plan{};
        if (!active || !levelRunning) return plan;
        if (levelInLayer < 3) return plan;

        int remainingBudget = saveItemTarget - saveItemsSpawned;
        if (remainingBudget <= 0) return plan;

        int remainingLevels = std::max(1, kLevelsPerLayer - std::max(3, levelInLayer) + 1);
        plan.eligible = true;
        plan.mustSpawn = remainingBudget >= remainingLevels;
        return plan;
    }

    void WriteLevelStartNotice(std::wostream& out) const {
        out << L"Layer " << layer << L" - Level " << levelInLayer;
        if (CurrentLevelHasBossEncounter()) out << L"  Encounter active";
    }

    void WriteSavedRunFields(std::wostream& out) const {
        const PlayableLevelSpec& spec = currentLevel;
        out << L"Layer=" << layer << L"\n";
        out << L"Level=" << levelInLayer << L"\n";
        out << L"CompletedLevels=" << completedLevels << L"\n";
        out << L"DarkLayerOne=" << (darkLayerOne ? 1 : 0) << L"\n";
        out << L"CustomGame=" << (customGame ? 1 : 0) << L"\n";
        out << L"CustomBrokenLampScares=" << (customSpec.brokenLampScares ? 1 : 0) << L"\n";
        out << L"CustomAirVentScares=" << (customSpec.airVentScares ? 1 : 0) << L"\n";
        out << L"CustomWaterScares=" << (customSpec.waterScares ? 1 : 0) << L"\n";
        out << L"CustomBloodWorldScares=" << (customSpec.bloodWorldScares ? 1 : 0) << L"\n";
        out << L"CustomFleshWorldScares=" << (customSpec.fleshWorldScares ? 1 : 0) << L"\n";
        out << L"CustomOmukadeBoss=" << (customSpec.omukadeBoss ? 1 : 0) << L"\n";
        out << L"CustomEightPages=" << (customSpec.eightPages ? 1 : 0) << L"\n";
        out << L"CustomRoomCount=" << customSpec.roomCount << L"\n";
        out << L"CustomJumpscareChancePercent=" << customSpec.jumpscareChancePercent << L"\n";
        out << L"CustomJumpscareStartMinSeconds=" << customSpec.jumpscareStartMinSeconds << L"\n";
        out << L"CustomJumpscareStartMaxSeconds=" << customSpec.jumpscareStartMaxSeconds << L"\n";
        out << L"CustomScareStartDelaySeconds=" << customScareStartDelaySeconds << L"\n";
        for (size_t i = 0; i < CustomGameSpec::kScareTypeCount; ++i) {
            out << L"CustomScare" << i << L"ChancePercent=" << customSpec.scareChancePercent[i] << L"\n";
            out << L"CustomScare" << i << L"StartMinSeconds=" << customSpec.scareStartMinSeconds[i] << L"\n";
            out << L"CustomScare" << i << L"StartMaxSeconds=" << customSpec.scareStartMaxSeconds[i] << L"\n";
            out << L"CustomScare" << i << L"StartDelaySeconds=" << customScareStartDelayByTypeSeconds[i] << L"\n";
        }
        out << L"SaveItemTarget=" << saveItemTarget << L"\n";
        out << L"SaveItemsSpawned=" << saveItemsSpawned << L"\n";
        out << L"LayerPagesCollected=" << layerPagesCollected << L"\n";
        for (size_t i = 0; i < levelPageTargets.size(); ++i) {
            out << L"LevelPageTarget" << i << L"=" << levelPageTargets[i] << L"\n";
        }
        for (size_t i = 0; i < layerPageCollected.size(); ++i) {
            out << L"LayerPage" << i << L"Collected=" << static_cast<int>(layerPageCollected[i]) << L"\n";
        }
        out << L"RunSeconds=" << runSeconds << L"\n";
        out << L"LevelSeconds=" << levelSeconds << L"\n";
        out << L"TotalScore=" << totalScore << L"\n";
        out << L"SpecMazeWidth=" << spec.mazeWidth << L"\n";
        out << L"SpecMazeHeight=" << spec.mazeHeight << L"\n";
        out << L"SpecBoss=" << (spec.bossEncounter ? 1 : 0) << L"\n";
        out << L"SpecBossChance=" << spec.bossEncounterChance << L"\n";
        out << L"SpecScareTier=" << static_cast<int>(spec.scareTier) << L"\n";
    }

    static SavedRunRestoreState ReadSavedRunFields(const SavedRunKeyValues& values) {
        SavedRunRestoreState saved{};
        saved.layer = SavedRunInt(values, L"Layer", 1);
        saved.levelInLayer = SavedRunInt(values, L"Level", 1);
        saved.completedLevels = SavedRunInt(values, L"CompletedLevels", 0);
        saved.darkLayerOne = SavedRunInt(values, L"DarkLayerOne", 0) != 0;
        saved.customGame = SavedRunInt(values, L"CustomGame", 0) != 0;
        saved.customSpec.brokenLampScares = SavedRunInt(values, L"CustomBrokenLampScares", 1) != 0;
        saved.customSpec.airVentScares = SavedRunInt(values, L"CustomAirVentScares", 1) != 0;
        saved.customSpec.waterScares = SavedRunInt(values, L"CustomWaterScares", 1) != 0;
        saved.customSpec.bloodWorldScares = SavedRunInt(values, L"CustomBloodWorldScares", 1) != 0;
        saved.customSpec.fleshWorldScares = SavedRunInt(values, L"CustomFleshWorldScares", 1) != 0;
        saved.customSpec.omukadeBoss = SavedRunInt(values, L"CustomOmukadeBoss", 1) != 0;
        saved.customSpec.eightPages = SavedRunInt(values, L"CustomEightPages", 1) != 0;
        saved.customSpec.roomCount = SavedRunInt(values, L"CustomRoomCount", saved.customSpec.roomCount);
        saved.customSpec.jumpscareChancePercent = SavedRunInt(values, L"CustomJumpscareChancePercent", saved.customSpec.jumpscareChancePercent);
        saved.customSpec.jumpscareStartMinSeconds = SavedRunInt(values, L"CustomJumpscareStartMinSeconds", saved.customSpec.jumpscareStartMinSeconds);
        saved.customSpec.jumpscareStartMaxSeconds = SavedRunInt(values, L"CustomJumpscareStartMaxSeconds", saved.customSpec.jumpscareStartMaxSeconds);
        saved.customScareStartDelaySeconds = SavedRunFloat(values, L"CustomScareStartDelaySeconds", 0.0f);
        for (size_t i = 0; i < CustomGameSpec::kScareTypeCount; ++i) {
            std::wstring prefix = L"CustomScare" + std::to_wstring(i);
            saved.customSpec.scareChancePercent[i] = SavedRunInt(values, (prefix + L"ChancePercent").c_str(), saved.customSpec.scareChancePercent[i]);
            saved.customSpec.scareStartMinSeconds[i] = SavedRunInt(values, (prefix + L"StartMinSeconds").c_str(), saved.customSpec.scareStartMinSeconds[i]);
            saved.customSpec.scareStartMaxSeconds[i] = SavedRunInt(values, (prefix + L"StartMaxSeconds").c_str(), saved.customSpec.scareStartMaxSeconds[i]);
            saved.customScareStartDelayByTypeSeconds[i] = SavedRunFloat(values, (prefix + L"StartDelaySeconds").c_str(), saved.customScareStartDelaySeconds);
        }
        saved.saveItemTarget = SavedRunInt(values, L"SaveItemTarget", 1);
        saved.saveItemsSpawned = SavedRunInt(values, L"SaveItemsSpawned", 0);
        for (size_t i = 0; i < saved.levelPageTargets.size(); ++i) {
            std::wstring key = L"LevelPageTarget" + std::to_wstring(i);
            saved.levelPageTargets[i] = SavedRunInt(values, key.c_str(), saved.levelPageTargets[i]);
        }
        saved.layerPagesCollected = SavedRunInt(values, L"LayerPagesCollected", 0);
        for (size_t i = 0; i < saved.layerPageCollected.size(); ++i) {
            std::wstring key = L"LayerPage" + std::to_wstring(i) + L"Collected";
            saved.layerPageCollected[i] = SavedRunInt(values, key.c_str(), 0) != 0 ? 1 : 0;
        }
        saved.runSeconds = SavedRunFloat(values, L"RunSeconds", 0.0f);
        saved.levelSeconds = SavedRunFloat(values, L"LevelSeconds", 0.0f);
        saved.totalScore = SavedRunInt(values, L"TotalScore", 0);
        saved.currentLevel.layer = saved.layer;
        saved.currentLevel.levelInLayer = saved.levelInLayer;
        saved.currentLevel.mazeWidth = std::clamp(SavedRunInt(values, L"SpecMazeWidth", 25), 3, 151);
        saved.currentLevel.mazeHeight = std::clamp(SavedRunInt(values, L"SpecMazeHeight", 25), 3, 151);
        saved.currentLevel.bossEncounter = SavedRunInt(values, L"SpecBoss", 0) != 0;
        saved.currentLevel.bossEncounterChance = std::clamp(SavedRunFloat(values, L"SpecBossChance", 0.0f), 0.0f, 1.0f);
        saved.currentLevel.scareTier = static_cast<PlayableScareTier>(
            std::clamp(SavedRunInt(values, L"SpecScareTier", 0), 0, 4));
        return saved;
    }

    static int PickSaveItemTarget(std::mt19937& rng) {
        return 1 + static_cast<int>(rng() % 3u);
    }

    static float RandRange(float a, float b, std::mt19937& rng) {
        std::uniform_real_distribution<float> dist(a, b);
        return dist(rng);
    }

    PlayableLevelSpec BuildLayerOneLevelSpec(int targetLevelInLayer, std::mt19937& rng) const {
        PlayableLevelSpec spec{};
        spec.layer = layer;
        spec.levelInLayer = std::clamp(targetLevelInLayer, 1, kLevelsPerLayer);

        switch (spec.levelInLayer) {
        case 1:
            spec.mazeWidth = 15;
            spec.mazeHeight = 15;
            spec.scareTier = PlayableScareTier::None;
            spec.bossEncounterChance = 0.0f;
            break;
        case 2:
            if ((rng() & 1u) == 0) {
                spec.mazeWidth = 20;
                spec.mazeHeight = 10;
            } else {
                spec.mazeWidth = 10;
                spec.mazeHeight = 20;
            }
            spec.scareTier = PlayableScareTier::Harmless;
            spec.bossEncounterChance = 0.05f;
            break;
        case 3:
            spec.mazeWidth = 25;
            spec.mazeHeight = 25;
            spec.scareTier = PlayableScareTier::Water;
            spec.bossEncounterChance = 0.15f;
            break;
        case 4:
            if ((rng() & 1u) == 0) {
                spec.mazeWidth = 30;
                spec.mazeHeight = 15;
            } else {
                spec.mazeWidth = 15;
                spec.mazeHeight = 30;
            }
            spec.scareTier = PlayableScareTier::Blood;
            spec.bossEncounterChance = 0.25f;
            break;
        default:
            {
                const std::array<Tile, 5> choices = {{{35, 20}, {40, 15}, {15, 40}, {35, 20}, {27, 27}}};
                Tile size = choices[static_cast<size_t>(rng() % choices.size())];
                spec.mazeWidth = size.x;
                spec.mazeHeight = size.y;
            }
            spec.scareTier = PlayableScareTier::Flesh;
            spec.bossEncounter = true;
            spec.bossEncounterChance = 1.0f;
            break;
        }

        if (!spec.bossEncounter && spec.bossEncounterChance > 0.0f) {
            float roll = static_cast<float>(rng() & 0xffffu) / 65535.0f;
            spec.bossEncounter = roll < spec.bossEncounterChance;
        }
        return spec;
    }

    float MapDirtProgression() const {
        if (!active) return 0.48f;
        if (customGame) {
            return Clamp01(static_cast<float>(customSpec.mapDirtPercent) / 100.0f);
        }

        float levelProgress = Clamp01((static_cast<float>(levelInLayer) - 1.0f) /
            static_cast<float>(std::max(1, kLevelsPerLayer - 1)));
        float layerProgress = Clamp01(static_cast<float>(std::max(0, layer - 1)) * 0.18f);
        return Clamp01(levelProgress + layerProgress);
    }

    float AirParticleDensityScale() const {
        if (!active) return 1.0f;
        int clampedLevel = std::clamp(levelInLayer, 1, kLevelsPerLayer);
        constexpr float kLevelScale[kLevelsPerLayer] = {
            0.16f, 0.34f, 0.56f, 0.78f, 1.0f
        };
        return kLevelScale[static_cast<size_t>(clampedLevel - 1)];
    }

    PlayableCustomScareGate CustomScareGateFor(int index) const {
        PlayableCustomScareGate gate{};
        if (!customGame) return gate;

        index = std::clamp(index, 0, CustomGameSpec::kScareTypeCount - 1);
        size_t slot = static_cast<size_t>(index);
        if (levelSeconds < customScareStartDelayByTypeSeconds[slot]) {
            gate.allowed = false;
            gate.requiresRoll = false;
            gate.chance = 0.0f;
            return gate;
        }

        gate.allowed = true;
        gate.requiresRoll = true;
        gate.chance = Clamp01(static_cast<float>(
            std::clamp(customSpec.scareChancePercent[slot], 0, 100)) / 100.0f);
        return gate;
    }

    void BeginLayerRun(bool useDarkLayerOne, std::mt19937& rng) {
        Reset();
        active = true;
        levelRunning = false;
        runFinished = false;
        layer = 1;
        levelInLayer = 1;
        darkLayerOne = useDarkLayerOne;
        saveItemTarget = PickSaveItemTarget(rng);
        GenerateLayerPageDistribution(rng);
        completed.reserve(kLevelsPerLayer);
    }

    void BeginCustomRun(const CustomGameSpec& spec, std::mt19937& rng) {
        Reset();
        active = true;
        levelRunning = true;
        runFinished = false;
        customGame = true;
        customSpec = spec;
        layer = spec.layer;
        levelInLayer = 1;
        darkLayerOne = false;
        saveItemTarget = PickSaveItemTarget(rng);
        completed.reserve(1);
        DisableLayerPages();
        if (spec.eightPages) levelPageTargets[0] = kCollectiblePageMaterialCount;
        customScareStartDelaySeconds = RandRange(
            static_cast<float>(spec.jumpscareStartMinSeconds),
            static_cast<float>(spec.jumpscareStartMaxSeconds),
            rng);
        for (size_t i = 0; i < CustomGameSpec::kScareTypeCount; ++i) {
            customScareStartDelayByTypeSeconds[i] = RandRange(
                static_cast<float>(spec.scareStartMinSeconds[i]),
                static_cast<float>(spec.scareStartMaxSeconds[i]),
                rng);
        }
    }

    void BeginLevel(const PlayableLevelSpec& spec) {
        levelInLayer = std::clamp(spec.levelInLayer, 1, kLevelsPerLayer);
        currentLevel = spec;
        levelSeconds = 0.0f;
        levelRunning = true;
        scoreScreenActive = false;
        scoreScreenFinal = false;
    }

    int RestoreSaveItemsSpawned(int spawned) {
        saveItemsSpawned = std::clamp(spawned, 0, saveItemTarget);
        return saveItemsSpawned;
    }

    int RestoreSavedRunState(SavedRunRestoreState saved) {
        Reset();
        active = true;
        levelRunning = true;
        runFinished = false;
        scoreScreenActive = false;
        scoreScreenFinal = false;
        layer = std::max(1, saved.layer);
        levelInLayer = std::clamp(saved.levelInLayer, 1, kLevelsPerLayer);
        completedLevels = std::max(0, saved.completedLevels);
        darkLayerOne = saved.darkLayerOne;
        customGame = saved.customGame;
        customSpec = saved.customSpec;
        customSpec.Normalize();
        customScareStartDelaySeconds = std::clamp(saved.customScareStartDelaySeconds, 0.0f, 600.0f);
        for (size_t i = 0; i < customScareStartDelayByTypeSeconds.size(); ++i) {
            customScareStartDelayByTypeSeconds[i] = std::clamp(
                saved.customScareStartDelayByTypeSeconds[i],
                0.0f,
                600.0f);
        }
        saveItemTarget = std::clamp(saved.saveItemTarget, 1, 3);
        int restoredSaveItemsSpawned = RestoreSaveItemsSpawned(saved.saveItemsSpawned);
        for (size_t i = 0; i < levelPageTargets.size(); ++i) {
            levelPageTargets[i] = std::clamp(saved.levelPageTargets[i], 0, kCollectiblePageMaterialCount);
        }
        EnsureLayerPageDistribution();
        SetLayerPagesCollectedCount(saved.layerPagesCollected);
        for (size_t i = 0; i < layerPageCollected.size(); ++i) {
            RestoreLayerPageCollected(static_cast<int>(i), saved.layerPageCollected[i] != 0);
        }
        if (customGame && !customSpec.eightPages) {
            DisableLayerPages();
        }
        runSeconds = std::max(0.0f, saved.runSeconds);
        levelSeconds = std::max(0.0f, saved.levelSeconds);
        totalScore = std::max(0, saved.totalScore);
        currentLevel = saved.currentLevel;
        currentLevel.layer = layer;
        currentLevel.levelInLayer = levelInLayer;
        if (customGame) {
            customSpec.layer = currentLevel.layer;
            customSpec.mazeWidth = currentLevel.mazeWidth;
            customSpec.mazeHeight = currentLevel.mazeHeight;
        }
        return restoredSaveItemsSpawned;
    }

    void GenerateLayerPageDistribution(std::mt19937& rng) {
        levelPageTargets.fill(1);
        std::array<int, kLevelsPerLayer> order{{0, 1, 2, 3, 4}};
        std::shuffle(order.begin(), order.end(), rng);
        for (int i = 0; i < 3; ++i) {
            levelPageTargets[static_cast<size_t>(order[static_cast<size_t>(i)])] = 2;
        }
        layerPageCollected.fill(0);
        layerPagesCollected = 0;
    }

    void EnsureLayerPageDistribution() {
        int total = 0;
        for (int count : levelPageTargets) total += count;
        if (total == kCollectiblePageMaterialCount) return;
        levelPageTargets = {{2, 2, 2, 1, 1}};
    }

    void DisableLayerPages() {
        layerPagesCollected = 0;
        layerPageCollected.fill(0);
        levelPageTargets.fill(0);
    }

    int LayerSecretTotal() const {
        return (customGame && !customSpec.eightPages) ? 0 : kCollectiblePageMaterialCount;
    }

    bool IsLayerPageCollected(int pageIndex) const {
        return pageIndex >= 0 && pageIndex < kCollectiblePageMaterialCount &&
            layerPageCollected[static_cast<size_t>(pageIndex)] != 0;
    }

    bool MarkLayerPageCollected(int pageIndex) {
        if (LayerSecretTotal() <= 0 || pageIndex < 0 || pageIndex >= kCollectiblePageMaterialCount) return false;

        uint8_t& collected = layerPageCollected[static_cast<size_t>(pageIndex)];
        if (collected != 0) return false;

        collected = 1;
        layerPagesCollected = std::clamp(layerPagesCollected + 1, 0, LayerSecretTotal());
        return true;
    }

    void RestoreLayerPageCollected(int pageIndex, bool collected) {
        if (pageIndex < 0 || pageIndex >= kCollectiblePageMaterialCount) return;
        if (LayerSecretTotal() <= 0) {
            layerPageCollected[static_cast<size_t>(pageIndex)] = 0;
            layerPagesCollected = 0;
            return;
        }
        layerPageCollected[static_cast<size_t>(pageIndex)] = collected ? 1 : 0;
    }

    void SetLayerPagesCollectedCount(int collectedCount) {
        layerPagesCollected = std::clamp(collectedCount, 0, LayerSecretTotal());
    }

    void ReconcileLayerPagesCollected() {
        int total = LayerSecretTotal();
        if (total <= 0) {
            layerPageCollected.fill(0);
            layerPagesCollected = 0;
            return;
        }

        int counted = 0;
        for (uint8_t collectedPage : layerPageCollected) {
            if (collectedPage) ++counted;
        }
        layerPagesCollected = std::clamp(std::max(layerPagesCollected, counted), 0, total);
    }

    int LayerPageStartForLevel(int targetLevelInLayer) const {
        int clamped = std::clamp(targetLevelInLayer, 1, kLevelsPerLayer);
        int start = 0;
        for (int i = 1; i < clamped; ++i) {
            start += levelPageTargets[static_cast<size_t>(i - 1)];
        }
        return std::clamp(start, 0, kCollectiblePageMaterialCount);
    }

    int LayerPageCountForLevel(int targetLevelInLayer) const {
        int index = std::clamp(targetLevelInLayer, 1, kLevelsPerLayer) - 1;
        int start = LayerPageStartForLevel(targetLevelInLayer);
        return std::clamp(levelPageTargets[static_cast<size_t>(index)], 0, kCollectiblePageMaterialCount - start);
    }

    int LayerPagesCollectedForLevel(int targetLevelInLayer) const {
        int start = LayerPageStartForLevel(targetLevelInLayer);
        int count = LayerPageCountForLevel(targetLevelInLayer);
        int found = 0;
        for (int i = 0; i < count; ++i) {
            int pageIndex = start + i;
            if (IsLayerPageCollected(pageIndex)) {
                ++found;
            }
        }
        return found;
    }

    PlayableLevelCompletionUpdate CompleteCurrentLevel(int score) {
        PlayableLevelCompletionUpdate update{};
        levelRunning = false;

        update.result.layer = layer;
        update.result.levelInLayer = levelInLayer;
        update.result.levelSeconds = levelSeconds;
        update.result.runSeconds = runSeconds;
        update.result.bossEncounter = currentLevel.bossEncounter;
        update.result.score = score;

        totalScore += update.result.score;
        lastResult = update.result;
        completed.push_back(update.result);
        completedLevels = static_cast<int>(completed.size());
        scoreScreenActive = true;

        ReconcileLayerPagesCollected();

        update.levelSecretTotal = LayerPageCountForLevel(update.result.levelInLayer);
        update.levelSecretsFound = update.levelSecretTotal > 0
            ? std::clamp(LayerPagesCollectedForLevel(update.result.levelInLayer), 0, update.levelSecretTotal)
            : 0;

        update.finalRun = customGame || levelInLayer >= kLevelsPerLayer;
        if (update.finalRun) {
            runFinished = true;
            active = false;
            scoreScreenFinal = true;
        }
        return update;
    }

    bool CanContinueAfterScoreScreen() const {
        return scoreScreenActive && !scoreScreenFinal;
    }

    int NextLevelAfterScoreScreen() const {
        return std::clamp(levelInLayer + 1, 1, kLevelsPerLayer);
    }

    void AdvanceRunningTimers(float dt) {
        if (!active || !levelRunning) return;
        float step = std::max(0.0f, dt);
        runSeconds += step;
        levelSeconds += step;
    }
};
