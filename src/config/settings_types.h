#pragma once

#include "../core/constants.h"

#include <array>
#include <cstdint>
#include <string>

// Settings data model.

struct Settings {
    bool allowWarpFallback = false;
    bool gameFullscreen = true;
    int gameResolutionWidth = 1920;
    int gameResolutionHeight = 1080;

    int mazeWidth = kMazeW;
    int mazeHeight = kMazeH;
    int roomCount = 3;
    int roomMinRadius = 1;
    int roomMaxRadius = 3;
    int roomCountRange = 1;
    int roomMinRadiusRange = 1;
    int roomMaxRadiusRange = 1;
    float extraConnectorMinRatio = 0.015f;
    float extraConnectorMaxRatio = 0.050f;
    int wallFeatureFrequency = 20;
    float wallFeatureFrequencySpread = 1.0f;
    int saveItemMinPerLayer = 1;
    int saveItemMaxPerLayer = 3;
    float saveItemLevelChance = 0.15f;
    uint32_t mazeSeed = 0;
    bool mapOverlay = true;
    bool debugAiMapOverlay = false;
    bool debugInfiniteStamina = false;
    bool debugInvincible = false;
    float runVariation = 0.1f;
    float tileWidthMeters = kTile;
    float tileLengthMeters = kTile;
    float wallHeightMeters = kWallH;

    float floorTextureMeters = kFloorTextureMeters;
    float wallTextureMeters = kWallTextureMeters;
    float ceilingTextureMeters = kCeilingTextureMeters;
    std::wstring assetFolder = L"assets\\PBRs";
    std::wstring wallStem = L"backrooms_wall";
    std::wstring floorStem = L"downloads\\Fabric029_4K-JPG\\Fabric029_4K-JPG";
    std::wstring ceilingStem = L"downloads\\OfficeCeiling001_4K-JPG\\OfficeCeiling001_4K-JPG";
    std::wstring fleshStem = L"downloads\\Others001_4k\\others_0001";
    bool useExternalNormals = true;
    int maxNormalMapMB = 512;

    float flashlightIntensity = 1.0f;
    float flashlightAttenuation = 0.115f;
    float flashlightConeDegrees = 80.0f;
    bool flashlightShadows = true;
    float flashlightShadowStrength = 0.72f;
    float flashlightShadowDistanceMeters = 18.0f;
    float flashlightShadowBias = 0.00075f;
    int flashlightShadowMapSize = 2048;
    float ambientLight = 0.0f;
    float lampIntensity = 1.45f;
    float lampSpacing = 3.2f;
    float lampOnRatio = 0.95f;
    float lampFlickerRatio = 0.10f;
    float brokenZoneRatio = 0.05f;
    float darkLampVisibleRatio = 1.0f;
    float fogStartMeters = 0.0f;
    float fogEndMeters = 28.0f;
    float fogDarkness = 1.0f;
    float cornerAOIntensity = 0.45f;
    float cornerAORadius = 0.5f;
    float floorCeilingAOIntensity = 0.3f;
    float exposure = 1.0f;
    float gamma = 1.0f;
    float motionBlurAmount = 0.18f;
    float bloomAmount = 0.22f;
    float lensDirtAmount = 0.18f;

    float walkSpeed = 1.692f;
    float roomSpeed = 1.45f;
    float runSpeed = 3.05f;
    float turnLookAheadTiles = 2.1f;
    float roomLookAheadTiles = 2.4f;
    float roomPauseChance = 0.62f;
    float junctionScanChance = 0.88f;
    float scanAngleDegrees = 55.0f;
    float lookBackMinSeconds = 5.0f;
    float lookBackMaxSeconds = 90.0f;
    float headBobAmount = 0.0825f;
    float sideSwayAmount = 0.025f;
    float junctionScanBaseSeconds = 0.85f;
    float junctionScanBranchSeconds = 1.08f;

    float flashlightSwayAmount = 3.0f;
    float flashlightFollowSpeed = 2.0f;
    float flashlightPanicDartAmount = 3.0f;
    float exitDoorOpenSeconds = 1.75f;
    float exitStepSeconds = 1.85f;
    float exitFadeSeconds = 1.10f;
    float exitStepDistance = 2.35f;
    float fadeInSeconds = 1.25f;
    float mouseSensitivity = 1.0f;
    bool invertMouseY = false;
    std::array<int, kGameInputActionCount> gameKeyBindings = DefaultGameKeyBindings();

    bool audioMuted = false;
    float audioMasterVolume = 1.0f;
    float audioEffectsVolume = 1.0f;
    float audioAmbienceVolume = 1.0f;
    float audioMonsterVolume = 1.0f;

    float paperDensity = 1.0f;
    float hallwayPaperRunDensity = 1.0f;
    float chairDensity = 1.0f;
    bool waterDamageEnabled = false;
    float waterDamageDensity = 0.0f;
    float metalCabinetDensity = 0.85f;
    float jumpscareFrequency = 0.15f;
    bool brokenLampScaresEnabled = true;
    bool airVentScaresEnabled = true;
    bool sparkParticles = true;
    float sparkEmitterRatio = 0.15f;
    float sparkBurstMinSeconds = 2.8f;
    float sparkBurstMaxSeconds = 8.8f;
    int sparkMaxParticles = 160;
    float sparkSize = 1.0f;
    bool airParticles = true;
    float airParticleDensity = 1.0f;
    float airParticleSize = 1.0f;
    float airParticleBlur = 1.0f;
    float bloodSplatterDensity = 0.0f;
    int bloodBurstCount = 25;
    float bloodWetness = 0.995f;
    int bloodStreamCount = static_cast<int>(kEffectBloodStreamCount);
    float bloodStreamThickness = kEffectBloodStreamThickness;
    float bloodShaderQuality = kEffectBloodShaderQuality;
    bool bloodWorldFlicker = true;
    bool bloodWorldAlwaysOn = false;
    float bloodWorldCoverage = 1.0f;
    float bloodWorldFlickerMinSeconds = 1500.0f;
    float bloodWorldFlickerMaxSeconds = 4800.0f;
    float bloodWorldFlickerDuration = 0.35f;
    float bloodWorldFlickerIntensity = 1.0f;
    bool bloodStudyView = false;
    bool fleshFlicker = true;
    bool fleshAlwaysOn = false;
    float fleshWetness = 0.995f;
    float fleshParallaxScale = 0.22f;
    float fleshFlickerMinSeconds = 1500.0f;
    float fleshFlickerMaxSeconds = 4800.0f;
    float fleshFlickerDuration = 0.35f;
    float fleshFlickerIntensity = 1.0f;

    float effectBloodLoopSeconds = kEffectBloodLoopSeconds;
    float effectBloodFullSpreadAge = kEffectBloodFullSpreadAge;
    float effectWaterLoopSeconds = kEffectWaterLoopSeconds;
    float effectAirVentLoopSeconds = kEffectAirVentLoopSeconds;
    float effectBrokenLampLoopSeconds = kEffectBrokenLampLoopSeconds;
    float effectStaticLoopSeconds = kEffectStaticLoopSeconds;
    float effectBrokenLampSparkIntensityMin = kEffectBrokenLampSparkIntensity.min;
    float effectBrokenLampSparkIntensityMax = kEffectBrokenLampSparkIntensity.max;
    float effectBrokenLampChainIntensityScale = kEffectBrokenLampChainIntensityScale;
    int effectBrokenLampChainBurstsMin = kEffectBrokenLampChainBursts.min;
    int effectBrokenLampChainBurstsMax = kEffectBrokenLampChainBursts.max;
    float effectAirVentSteamIntensityMin = kEffectAirVentSteamIntensity.min;
    float effectAirVentSteamIntensityMax = kEffectAirVentSteamIntensity.max;
    int effectAirVentPanelDropEvery = kEffectAirVentPanelDropEvery;
    float effectAirVentPanelDropChance = 0.10f;

    bool dreadEnabled = true;
    bool dreadDebugMeter = false;
    float dreadDecayPerSecond = 0.030f;
    float dreadMonsterDistance = 13.0f;
    float dreadMonsterGainPerSecond = 0.42f;
    float dreadJumpscareGain = 0.1f;
    float dreadFleshGain = 0.2f;
    float dreadWalkSpeedBoost = 0.05f;
    float dreadRunSpeedBoost = 0.075f;
    float dreadFlashlightFlicker = 1.0f;

    float monsterScale = 1.0f;
    float monsterSpeed = 0.68f;
    float monsterSprintSpeed = 0.88f;
    bool monsterIgnorePlayer = false;
    float monsterKillDistance = 1.18f;
    float monsterVisibleDistance = 12.0f;
    std::wstring monsterSkullMesh = L"assets\\models\\monster_face_mask\\horror_mask.obj";
    std::wstring monsterAltSkullMesh = L"";
    float monsterAltSkullChance = 0.0f;
    int monsterSkullMaxTriangles = 16000;
    float monsterSkullYawDegrees = 0.0f;
    float monsterSkullPitchDegrees = 0.0f;
    float monsterSkullRollDegrees = 0.0f;
    float monsterAltSkullYawDegrees = -90.0f;
    float monsterAltSkullPitchDegrees = 0.0f;
    float monsterAltSkullRollDegrees = 90.0f;
};

