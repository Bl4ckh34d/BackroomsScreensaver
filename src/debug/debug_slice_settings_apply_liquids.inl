        settingsRuntime_.live.waterDamageEnabled = DebugSliceEffectIsWater(gDebugSliceEffect);
        settingsRuntime_.live.waterDamageDensity = settingsRuntime_.live.waterDamageEnabled ? 1.0f : 0.0f;
        settingsRuntime_.live.bloodSplatterDensity = 0.0f;
        settingsRuntime_.live.bloodBurstCount = 0;
        settingsRuntime_.live.bloodStreamCount = liquidDebug ? 14 : 0;
        settingsRuntime_.live.bloodStreamThickness = std::max(settingsRuntime_.live.bloodStreamThickness, 0.88f);
        settingsRuntime_.live.bloodShaderQuality = liquidDebug ? 0.42f : 1.0f;
