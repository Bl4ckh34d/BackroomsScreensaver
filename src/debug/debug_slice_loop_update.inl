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
