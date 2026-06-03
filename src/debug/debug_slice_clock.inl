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
