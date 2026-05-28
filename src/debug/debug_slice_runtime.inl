    void ApplyDebugSliceSettings() {
        gDebugSliceTiles = std::clamp(gDebugSliceTiles, 1, 5);
        if (gDebugSliceEffect == DebugSliceEffect::Props) {
            gDebugSliceTiles = std::max(gDebugSliceTiles, 3);
        }
        bool liquidDebug = gDebugSliceEffect == DebugSliceEffect::Blood || DebugSliceEffectIsWater(gDebugSliceEffect);
        gBloodDebugEveryWall = gEffectDebugViewer && liquidDebug;

        settings_.mazeWidth = gDebugSliceTiles + 2;
        settings_.mazeHeight = gDebugSliceTiles + 2;
        settings_.roomCount = 0;
        settings_.roomCountRange = 0;
        settings_.roomMinRadiusRange = 0;
        settings_.roomMaxRadiusRange = 0;
        settings_.runVariation = 0.0f;
        settings_.mapOverlay = false;
        settings_.ambientLight = kEffectDebugAmbientLight;
        settings_.exposure = std::min(settings_.exposure, kEffectDebugExposureMax);
        settings_.bloomAmount = std::min(settings_.bloomAmount, kEffectDebugBloomMax);
        settings_.fogStartMeters = 1000.0f;
        settings_.fogEndMeters = 1100.0f;
        settings_.fogDarkness = 0.0f;
        settings_.flashlightIntensity = kEffectDebugFlashlightIntensity;
        settings_.flashlightShadows = false;
        settings_.paperDensity = 0.0f;
        settings_.hallwayPaperRunDensity = 0.0f;
        settings_.chairDensity = 0.0f;
        settings_.metalCabinetDensity = 0.0f;
        settings_.jumpscareFrequency = 0.0f;
        settings_.airParticles = gDebugSliceEffect == DebugSliceEffect::AirVents;
        settings_.sparkParticles = gDebugSliceEffect == DebugSliceEffect::BrokenLamps;
        settings_.bloodStudyView = false;
        settings_.bloodWorldFlicker = false;
        settings_.bloodWorldAlwaysOn = false;
        settings_.bloodWorldCoverage = 0.0f;
        settings_.fleshFlicker = false;

        settings_.waterDamageDensity = 0.0f;
        settings_.bloodSplatterDensity = 0.0f;
        settings_.bloodBurstCount = 0;
        settings_.bloodStreamCount = liquidDebug ? 14 : 0;
        settings_.bloodStreamThickness = std::max(settings_.bloodStreamThickness, 0.88f);
        settings_.bloodShaderQuality = liquidDebug ? 0.42f : 1.0f;

        settings_.lampSpacing = settings_.tileWidthMeters;
        settings_.lampIntensity = (gDebugSliceEffect == DebugSliceEffect::CeilingLamps) ? kEffectDebugLampIntensity : 0.0f;
        settings_.lampFlickerRatio = 0.0f;
        if (gDebugSliceEffect == DebugSliceEffect::CeilingLamps) {
            settings_.lampOnRatio = 1.0f;
            settings_.darkLampVisibleRatio = 1.0f;
            settings_.brokenZoneRatio = 0.0f;
        } else if (gDebugSliceEffect == DebugSliceEffect::BrokenLamps) {
            settings_.lampOnRatio = 0.0f;
            settings_.darkLampVisibleRatio = 0.0f;
            settings_.brokenZoneRatio = 1.0f;
            settings_.sparkEmitterRatio = 1.0f;
        } else {
            settings_.lampOnRatio = 0.0f;
            settings_.darkLampVisibleRatio = 0.0f;
            settings_.brokenZoneRatio = 0.0f;
        }
        if (gDebugSliceEffect == DebugSliceEffect::Props) {
            settings_.ambientLight = std::max(settings_.ambientLight, 0.42f);
            settings_.flashlightIntensity = std::max(settings_.flashlightIntensity, 2.4f);
        }
    }

    float DebugSliceClockSeconds() const {
        if (!gEffectDebugViewer || bloodDebugStartTicks_ == 0) return time_;
        return static_cast<float>(GetTickCount64() - bloodDebugStartTicks_) / 1000.0f;
    }

    float DebugSliceLoopSeconds() const {
        if (gDebugSliceEffect == DebugSliceEffect::Blood) return settings_.effectBloodLoopSeconds;
        if (DebugSliceEffectIsWater(gDebugSliceEffect)) return std::max(settings_.effectWaterLoopSeconds, settings_.effectBloodLoopSeconds * 0.82f);
        if (gDebugSliceEffect == DebugSliceEffect::AirVents) return settings_.effectAirVentLoopSeconds;
        if (gDebugSliceEffect == DebugSliceEffect::BrokenLamps) return settings_.effectBrokenLampLoopSeconds;
        return settings_.effectStaticLoopSeconds;
    }

    float DebugSliceLoopPhase() const {
        float loopSeconds = std::max(0.1f, DebugSliceLoopSeconds());
        return std::fmod(std::max(0.0f, DebugSliceClockSeconds()), loopSeconds) / loopSeconds;
    }

    void ResetDebugSliceLoopState() {
        debugSliceLoopCycle_ = -1;
        sparks_.clear();
        sparkFlashes_.clear();
        sparkChains_.clear();
        steam_.clear();
        ventDrops_.clear();
        for (SparkEmitter& emitter : sparkEmitters_) {
            emitter.triggered = false;
        }
        for (SteamEmitter& emitter : steamEmitters_) {
            emitter.triggered = false;
            emitter.panelDropped = false;
        }
    }

    void UpdateDebugSliceLoop(float) {
        if (!gEffectDebugViewer) return;
        float loopSeconds = std::max(0.1f, DebugSliceLoopSeconds());
        int cycle = static_cast<int>(std::floor(DebugSliceClockSeconds() / loopSeconds));
        if (cycle == debugSliceLoopCycle_) return;
        debugSliceLoopCycle_ = cycle;

        if (gDebugSliceEffect == DebugSliceEffect::BrokenLamps) {
            sparks_.clear();
            sparkFlashes_.clear();
            sparkChains_.clear();
            for (SparkEmitter& emitter : sparkEmitters_) {
                emitter.triggered = false;
                float intensity = PickBrokenLampSparkIntensity();
                int chainBursts = PickBrokenLampChainBursts();
                SpawnSparkBurst(emitter, intensity);
                ScheduleSparkChain(emitter.pos, intensity * settings_.effectBrokenLampChainIntensityScale, chainBursts);
            }
        } else if (gDebugSliceEffect == DebugSliceEffect::AirVents) {
            steam_.clear();
            sparks_.clear();
            sparkFlashes_.clear();
            sparkChains_.clear();
            int emitterIndex = 0;
            for (SteamEmitter& emitter : steamEmitters_) {
                emitter.triggered = false;
                SpawnSteamBurst(emitter, PickAirVentSteamIntensity());
                if (!emitter.panelDropped &&
                    ((emitterIndex + cycle) % std::max(1, settings_.effectAirVentPanelDropEvery)) == 0 &&
                    SpawnVentDrop(emitter)) {
                    emitter.panelDropped = true;
                }
                ++emitterIndex;
            }
        }
    }
