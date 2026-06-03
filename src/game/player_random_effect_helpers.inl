// Runtime random and effect tuning helpers. 
// Included inside Renderer's private section from player_camera_movement.inl.

    float RandRange(float a, float b) {
        std::uniform_real_distribution<float> dist(a, b);
        return dist(sessionRuntime_.rng);
    }

    float RandEffectRange(float a, float b) {
        if (b < a) std::swap(a, b);
        return RandRange(a, b);
    }

    int RandEffectRangeInt(int a, int b) {
        if (b < a) std::swap(a, b);
        if (b <= a) return a;
        int value = a + static_cast<int>(RandRange(0.0f, static_cast<float>(b - a + 1)));
        return std::clamp(value, a, b);
    }

    float PickBrokenLampSparkIntensity() {
        return RandEffectRange(settingsRuntime_.live.effectBrokenLampSparkIntensityMin, settingsRuntime_.live.effectBrokenLampSparkIntensityMax);
    }

    int PickBrokenLampChainBursts() {
        return RandEffectRangeInt(settingsRuntime_.live.effectBrokenLampChainBurstsMin, settingsRuntime_.live.effectBrokenLampChainBurstsMax);
    }

    float PickAirVentSteamIntensity() {
        return RandEffectRange(settingsRuntime_.live.effectAirVentSteamIntensityMin, settingsRuntime_.live.effectAirVentSteamIntensityMax);
    }

    float JumpscareFrequency() const {
        return Clamp01(settingsRuntime_.live.jumpscareFrequency);
    }

    float ScareCooldownScale() const {
        return 1.0f / std::max(0.08f, JumpscareFrequency());
    }

    float AmbientSparkCooldownScale() const {
        return Lerp(4.0f, 1.0f, JumpscareFrequency());
    }
