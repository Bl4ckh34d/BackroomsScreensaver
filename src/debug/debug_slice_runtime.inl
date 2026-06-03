    void ApplyDebugSliceSettings() {
        gDebugSliceTiles = std::clamp(gDebugSliceTiles, 1, 5);
        if (gDebugSliceEffect == DebugSliceEffect::Props) {
            gDebugSliceTiles = std::max(gDebugSliceTiles, 3);
        }
        bool liquidDebug = gDebugSliceEffect == DebugSliceEffect::Blood || DebugSliceEffectIsWater(gDebugSliceEffect);
        gBloodDebugEveryWall = gEffectDebugViewer && liquidDebug;

        settingsRuntime_.live.mazeWidth = gDebugSliceTiles + 2;
        settingsRuntime_.live.mazeHeight = gDebugSliceTiles + 2;
        settingsRuntime_.live.roomCount = 0;
        settingsRuntime_.live.roomCountRange = 0;
        settingsRuntime_.live.roomMinRadiusRange = 0;
        settingsRuntime_.live.roomMaxRadiusRange = 0;
        settingsRuntime_.live.runVariation = 0.0f;
        settingsRuntime_.live.mapOverlay = false;
        settingsRuntime_.live.ambientLight = kEffectDebugAmbientLight;
        settingsRuntime_.live.exposure = std::min(settingsRuntime_.live.exposure, kEffectDebugExposureMax);
        settingsRuntime_.live.bloomAmount = std::min(settingsRuntime_.live.bloomAmount, kEffectDebugBloomMax);
        settingsRuntime_.live.fogStartMeters = 1000.0f;
        settingsRuntime_.live.fogEndMeters = 1100.0f;
        settingsRuntime_.live.fogDarkness = 0.0f;
        settingsRuntime_.live.flashlightIntensity = kEffectDebugFlashlightIntensity;
        settingsRuntime_.live.flashlightShadows = false;
        settingsRuntime_.live.paperDensity = 0.0f;
        settingsRuntime_.live.hallwayPaperRunDensity = 0.0f;
        settingsRuntime_.live.chairDensity = 0.0f;
        settingsRuntime_.live.metalCabinetDensity = 0.0f;
        settingsRuntime_.live.jumpscareFrequency = 0.0f;
        settingsRuntime_.live.airParticles = gDebugSliceEffect == DebugSliceEffect::AirVents;
        settingsRuntime_.live.sparkParticles = gDebugSliceEffect == DebugSliceEffect::BrokenLamps;
        settingsRuntime_.live.bloodStudyView = false;
        settingsRuntime_.live.bloodWorldFlicker = false;
        settingsRuntime_.live.bloodWorldAlwaysOn = false;
        settingsRuntime_.live.bloodWorldCoverage = 0.0f;
        settingsRuntime_.live.fleshFlicker = false;

        settingsRuntime_.live.waterDamageEnabled = DebugSliceEffectIsWater(gDebugSliceEffect);
        settingsRuntime_.live.waterDamageDensity = settingsRuntime_.live.waterDamageEnabled ? 1.0f : 0.0f;
        settingsRuntime_.live.bloodSplatterDensity = 0.0f;
        settingsRuntime_.live.bloodBurstCount = 0;
        settingsRuntime_.live.bloodStreamCount = liquidDebug ? 14 : 0;
        settingsRuntime_.live.bloodStreamThickness = std::max(settingsRuntime_.live.bloodStreamThickness, 0.88f);
        settingsRuntime_.live.bloodShaderQuality = liquidDebug ? 0.42f : 1.0f;

        settingsRuntime_.live.lampSpacing = settingsRuntime_.live.tileWidthMeters;
        settingsRuntime_.live.lampIntensity = (gDebugSliceEffect == DebugSliceEffect::CeilingLamps) ? kEffectDebugLampIntensity : 0.0f;
        settingsRuntime_.live.lampFlickerRatio = 0.0f;
        if (gDebugSliceEffect == DebugSliceEffect::CeilingLamps) {
            settingsRuntime_.live.lampOnRatio = 1.0f;
            settingsRuntime_.live.darkLampVisibleRatio = 1.0f;
            settingsRuntime_.live.brokenZoneRatio = 0.0f;
        } else if (gDebugSliceEffect == DebugSliceEffect::BrokenLamps) {
            settingsRuntime_.live.lampOnRatio = 0.0f;
            settingsRuntime_.live.darkLampVisibleRatio = 0.0f;
            settingsRuntime_.live.brokenZoneRatio = 1.0f;
            settingsRuntime_.live.sparkEmitterRatio = 1.0f;
        } else {
            settingsRuntime_.live.lampOnRatio = 0.0f;
            settingsRuntime_.live.darkLampVisibleRatio = 0.0f;
            settingsRuntime_.live.brokenZoneRatio = 0.0f;
        }
        if (gDebugSliceEffect == DebugSliceEffect::Props) {
            settingsRuntime_.live.ambientLight = std::max(settingsRuntime_.live.ambientLight, 0.42f);
            settingsRuntime_.live.flashlightIntensity = std::max(settingsRuntime_.live.flashlightIntensity, 2.4f);
        }
    }

    float DebugSliceClockSeconds() const {
        if (!gEffectDebugViewer || debugRuntime_.bloodDebugStartTicks == 0) return timeRuntime_.time;
        return static_cast<float>(GetTickCount64() - debugRuntime_.bloodDebugStartTicks) / 1000.0f;
    }

    float DebugSliceLoopSeconds() const {
        if (gDebugSliceEffect == DebugSliceEffect::Blood) return settingsRuntime_.live.effectBloodLoopSeconds;
        if (DebugSliceEffectIsWater(gDebugSliceEffect)) return std::max(settingsRuntime_.live.effectWaterLoopSeconds, settingsRuntime_.live.effectBloodLoopSeconds * 0.82f);
        if (gDebugSliceEffect == DebugSliceEffect::AirVents) return settingsRuntime_.live.effectAirVentLoopSeconds;
        if (gDebugSliceEffect == DebugSliceEffect::BrokenLamps) return settingsRuntime_.live.effectBrokenLampLoopSeconds;
        return settingsRuntime_.live.effectStaticLoopSeconds;
    }

    float DebugSliceLoopPhase() const {
        float loopSeconds = std::max(0.1f, DebugSliceLoopSeconds());
        return std::fmod(std::max(0.0f, DebugSliceClockSeconds()), loopSeconds) / loopSeconds;
    }

    void ResetDebugSliceLoopState() {
        debugRuntime_.debugSliceLoopCycle = -1;
        effectRuntime_.sparks.clear();
        effectRuntime_.sparkFlashes.clear();
        effectRuntime_.sparkChains.clear();
        effectRuntime_.steam.clear();
        effectRuntime_.ventDrops.clear();
        for (SparkEmitter& emitter : effectRuntime_.sparkEmitters) {
            emitter.triggered = false;
        }
        for (SteamEmitter& emitter : effectRuntime_.steamEmitters) {
            emitter.triggered = false;
            emitter.panelDropped = false;
        }
    }

    void UpdateDebugSliceLoop(float) {
        if (!gEffectDebugViewer) return;
        float loopSeconds = std::max(0.1f, DebugSliceLoopSeconds());
        int cycle = static_cast<int>(std::floor(DebugSliceClockSeconds() / loopSeconds));
        if (cycle == debugRuntime_.debugSliceLoopCycle) return;
        debugRuntime_.debugSliceLoopCycle = cycle;

        if (gDebugSliceEffect == DebugSliceEffect::BrokenLamps) {
            effectRuntime_.sparks.clear();
            effectRuntime_.sparkFlashes.clear();
            effectRuntime_.sparkChains.clear();
            for (SparkEmitter& emitter : effectRuntime_.sparkEmitters) {
                emitter.triggered = false;
                float intensity = PickBrokenLampSparkIntensity();
                int chainBursts = PickBrokenLampChainBursts();
                SpawnSparkBurst(emitter, intensity);
                ScheduleSparkChain(emitter.pos, intensity * settingsRuntime_.live.effectBrokenLampChainIntensityScale, chainBursts);
            }
        } else if (gDebugSliceEffect == DebugSliceEffect::AirVents) {
            effectRuntime_.steam.clear();
            effectRuntime_.sparks.clear();
            effectRuntime_.sparkFlashes.clear();
            effectRuntime_.sparkChains.clear();
            int emitterIndex = 0;
            for (SteamEmitter& emitter : effectRuntime_.steamEmitters) {
                emitter.triggered = false;
                SpawnSteamBurst(emitter, PickAirVentSteamIntensity());
                if (!emitter.panelDropped &&
                    ((emitterIndex + cycle) % std::max(1, settingsRuntime_.live.effectAirVentPanelDropEvery)) == 0 &&
                    SpawnVentDrop(emitter)) {
                    emitter.panelDropped = true;
                }
                ++emitterIndex;
            }
        }
    }
