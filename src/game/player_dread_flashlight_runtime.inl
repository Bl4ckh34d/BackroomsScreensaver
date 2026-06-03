    float DreadPressure() const {
        if (!settingsRuntime_.live.dreadEnabled) return 0.0f;
        float proximity = Clamp01((settingsRuntime_.live.dreadMonsterDistance - MonsterDistance()) / std::max(0.1f, settingsRuntime_.live.dreadMonsterDistance));
        return Clamp01(std::max(viewRuntime_.dreadLevel * 0.86f, proximity * 0.74f));
    }

    float DreadFlashlightMultiplier() const {
        if (!settingsRuntime_.live.dreadEnabled) return 1.0f;
        float monsterRange = std::max(0.1f, settingsRuntime_.live.dreadMonsterDistance);
        float monsterProximity = Clamp01((monsterRange - MonsterDistance()) / monsterRange);
        float pressure = Clamp01(SmoothStep(0.10f, 1.0f, monsterProximity) * settingsRuntime_.live.dreadFlashlightFlicker);
        if (pressure <= 0.02f) return 1.0f;
        float waves =
            std::sin(timeRuntime_.time * 19.7f + std::sin(timeRuntime_.time * 3.1f) * 0.8f) +
            std::sin(timeRuntime_.time * 43.3f + 1.8f) * 0.58f +
            std::sin(timeRuntime_.time * 91.9f + 0.4f) * 0.28f;
        float gate = waves > (1.06f - pressure * 1.18f) ? 1.0f : 0.0f;
        float flutter = 0.5f + 0.5f * std::sin(timeRuntime_.time * (11.0f + pressure * 29.0f) + std::sin(timeRuntime_.time * 5.7f) * 1.4f);
        float drop = pressure * (0.05f + flutter * 0.12f + gate * (0.24f + pressure * 0.42f));
        return std::clamp(1.0f - drop, 0.18f, 1.05f);
    }
