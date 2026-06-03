        float dirtProgression = 0.48f;
        if (IsPlayableSimulationMode(sessionRuntime_.mode) && gameWorld_.PlayableRunActive()) {
            dirtProgression = gameWorld_.MapDirtProgression();
        } else if (sessionRuntime_.mode == RendererRuntimeMode::MainMenu) {
            dirtProgression = menuRuntime_.darkLayerOneRun ? 0.72f : 0.42f;
        } else if (gEffectDebugViewer && DebugSliceEffectIsWater(gDebugSliceEffect)) {
            dirtProgression = 0.90f;
        } else {
            float damagePressure = (settingsRuntime_.live.waterDamageEnabled ? 0.24f : 0.0f) +
                std::clamp(settingsRuntime_.live.waterDamageDensity, 0.0f, 4.0f) * 0.16f +
                std::clamp(settingsRuntime_.live.bloodSplatterDensity, 0.0f, 4.0f) * 0.12f +
                std::clamp(settingsRuntime_.live.jumpscareFrequency, 0.0f, 1.0f) * 0.12f +
                std::clamp(settingsRuntime_.live.brokenZoneRatio, 0.0f, 1.0f) * 0.18f;
            dirtProgression = Clamp01(0.34f + damagePressure);
        }
        cb.texture0 = {
            settingsRuntime_.live.wallTextureMeters,
            settingsRuntime_.live.floorTextureMeters,
            settingsRuntime_.live.ceilingTextureMeters,
            dirtProgression
        };
