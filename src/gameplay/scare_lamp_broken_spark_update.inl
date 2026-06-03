    void UpdateBrokenRuntimeLampSparks(float dt, float minSeconds, float maxSeconds, float chainChance) {
        if (dt <= 0.0f || !settingsRuntime_.live.sparkParticles || effectRuntime_.runtimeLamps.empty()) return;
        for (RuntimeLampState& lamp : effectRuntime_.runtimeLamps) {
            if (!lamp.broken) continue;
            if (!LampCanEmitSparks(lamp)) {
                lamp.sparkTimer = std::max(lamp.sparkTimer, maxSeconds);
                continue;
            }
            lamp.sparkTimer -= dt;
            if (lamp.sparkTimer > 0.0f) continue;

            float intensity = RandRange(0.42f, 1.28f);
            EmitSparkBurstAt(lamp.pos, intensity);
            if (RandRange(0.0f, 1.0f) < chainChance) {
                ScheduleSparkChain(lamp.pos, intensity * settingsRuntime_.live.effectBrokenLampChainIntensityScale,
                    std::max(1, PickBrokenLampChainBursts() / 2));
            }
            lamp.sparkTimer = RandRange(std::max(0.12f, minSeconds), std::max(minSeconds, maxSeconds));
        }
    }
