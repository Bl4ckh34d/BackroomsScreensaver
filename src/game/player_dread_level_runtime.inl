    void AddDread(float amount) {
        if (!settingsRuntime_.live.dreadEnabled || amount <= 0.0f) return;
        viewRuntime_.dreadLevel = Clamp01(viewRuntime_.dreadLevel + amount);
        viewRuntime_.flashlightAgitation = std::max(viewRuntime_.flashlightAgitation, std::min(1.0f, amount * 2.2f));
    }

    void UpdateDreadMeterDisplay(float dt) {
        if (!settingsRuntime_.live.dreadEnabled) {
            viewRuntime_.dreadMeterLevel = 0.0f;
            return;
        }
        float response = viewRuntime_.dreadLevel > viewRuntime_.dreadMeterLevel ? 18.0f : 5.5f;
        viewRuntime_.dreadMeterLevel += (viewRuntime_.dreadLevel - viewRuntime_.dreadMeterLevel) * std::min(1.0f, std::max(0.0f, dt) * response);
    }
