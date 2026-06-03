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
