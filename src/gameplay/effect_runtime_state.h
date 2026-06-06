#pragma once

struct EnvironmentalEffectRuntimeState {
    std::vector<uint8_t> wetFootstepTiles;
    std::vector<uint8_t> wetCeilingTiles;
    std::vector<WetDripEmitter> wetDripEmitters;
    std::vector<WetFloorFootprint> wetFloorFootprints;
    std::vector<AirParticle> airParticles;
    float airParticleBudgetScale = 1.0f;
    float airParticleFrameDt = 0.0f;
    int airParticleValidationCursor = 0;
    std::vector<SparkEmitter> sparkEmitters;
    std::vector<RuntimeLampState> runtimeLamps;
    std::vector<uint8_t> lampDamagePixels;
    std::vector<XMFLOAT4> bakedLampLightPixels;
    std::vector<SteamEmitter> steamEmitters;
    std::vector<SparkParticle> sparks;
    std::vector<SparkFlash> sparkFlashes;
    std::vector<SparkChain> sparkChains;
    std::vector<SteamParticle> steam;
    std::vector<VentDrop> ventDrops;
    float sparkCooldown = 3.0f;
    bool lampDamageDirty = false;
    bool bakedLampLightDirty = false;

    void ClearTransientParticles() {
        sparks.clear();
        sparkFlashes.clear();
        sparkChains.clear();
        steam.clear();
        ventDrops.clear();
    }
};

struct ScareEffectRuntimeState {
    std::vector<BloodScarePoint> bloodScarePoints;
    std::vector<BloodRevealRegion> bloodRevealRegions;
    int activeBloodScareIndex = -1;
    float bloodScareActiveUntil = 0.0f;
    float bloodWorldFlickerCooldown = 1500.0f;
    float bloodWorldFlickerTimer = 0.0f;
    float bloodWorldFlickerDuration = 1.0f;
    float bloodWorldActivationTime = -1000.0f;
    float bloodFocusTimer = 0.0f;
    float bloodFocusDuration = 0.0f;
    int bloodFocusReactionsTaken = 0;
    float bloodFocusReactionCooldown = 0.0f;
    float proximityBloodPulseCooldown = 0.0f;
    XMFLOAT3 bloodFocusTarget{};
    Tile bloodStudyTile{-1000, -1000};
    float scareCooldown = 2.0f;
    float fleshFlickerCooldown = 1500.0f;
    float fleshFlickerTimer = 0.0f;
    float fleshFlickerDuration = 1.0f;
    float visionFlashTimer = 0.0f;
    float visionFlashDuration = 0.16f;
    Tile scareEventTile{-1000, -1000};
};
