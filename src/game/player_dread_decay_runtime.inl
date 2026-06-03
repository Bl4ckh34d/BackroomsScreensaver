    void UpdateDread(float dt, bool threat, float monsterDist) {
        if (!settingsRuntime_.live.dreadEnabled) {
            viewRuntime_.dreadLevel = 0.0f;
            viewRuntime_.dreadMeterLevel = 0.0f;
            viewRuntime_.monsterSpottedLast = false;
            viewRuntime_.monsterSightDreadCooldown = 0.0f;
            return;
        }
        float decay = settingsRuntime_.live.dreadDecayPerSecond * dt * (threat ? 0.20f : 1.0f);
        viewRuntime_.dreadLevel = std::max(0.0f, viewRuntime_.dreadLevel - decay);

        float proximity = Clamp01((settingsRuntime_.live.dreadMonsterDistance - monsterDist) / std::max(0.1f, settingsRuntime_.live.dreadMonsterDistance));
        if (proximity > 0.0f) {
            float visibleWeight = threat ? 1.0f : 0.32f;
            float gain = std::pow(proximity, 1.45f) * settingsRuntime_.live.dreadMonsterGainPerSecond * visibleWeight * dt;
            viewRuntime_.dreadLevel = Clamp01(viewRuntime_.dreadLevel + gain);
            if (threat) {
                viewRuntime_.dreadLevel = std::max(viewRuntime_.dreadLevel, proximity * 0.58f);
            }
        }
    }
