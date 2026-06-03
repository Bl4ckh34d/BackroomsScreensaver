        float bloodStreamCount = static_cast<float>(std::clamp(settingsRuntime_.live.bloodStreamCount, 4, 32));
        float bloodStreamThickness = std::clamp(settingsRuntime_.live.bloodStreamThickness, 0.10f, 2.0f);
        float bloodShaderQuality = std::clamp(settingsRuntime_.live.bloodShaderQuality, 0.25f, 1.0f);
        float bloodWorldAmount = 0.0f;
        if (scareRuntime_.bloodWorldFlickerTimer > 0.0f && scareRuntime_.bloodWorldFlickerDuration > 0.001f) {
            float elapsed = scareRuntime_.bloodWorldFlickerDuration - scareRuntime_.bloodWorldFlickerTimer;
            float envelope = SmoothStep(0.0f, 0.055f, elapsed) *
                (1.0f - SmoothStep(scareRuntime_.bloodWorldFlickerDuration - 0.18f, scareRuntime_.bloodWorldFlickerDuration, elapsed));
            float strobe = ((std::sin(elapsed * 41.0f) + std::sin(elapsed * 93.0f) * 0.48f + std::sin(elapsed * 151.0f) * 0.22f) > -0.06f) ? 1.0f : 0.0f;
            bloodWorldAmount = envelope * strobe * settingsRuntime_.live.bloodWorldFlickerIntensity;
        }
        if (settingsRuntime_.live.bloodWorldAlwaysOn && settingsRuntime_.live.bloodWorldCoverage > 0.001f) {
            bloodWorldAmount = std::max(bloodWorldAmount, settingsRuntime_.live.bloodWorldFlickerIntensity);
            if (scareRuntime_.bloodWorldActivationTime < -900.0f) scareRuntime_.bloodWorldActivationTime = timeRuntime_.time - 46.0f;
        }
        if (sessionRuntime_.mode == RendererRuntimeMode::MainMenu && settingsRuntime_.live.bloodWorldCoverage > 0.001f) {
            bloodWorldAmount = std::max(bloodWorldAmount, menuRuntime_.bloodAmount);
        }
