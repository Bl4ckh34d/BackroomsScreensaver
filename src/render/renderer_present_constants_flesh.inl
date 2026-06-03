// Scene constants for flesh flicker, blood world, blood reveal regions, and blood shader tuning.

        float fleshAmount = 0.0f;
        if (scareRuntime_.fleshFlickerTimer > 0.0f && scareRuntime_.fleshFlickerDuration > 0.001f) {
            float elapsed = scareRuntime_.fleshFlickerDuration - scareRuntime_.fleshFlickerTimer;
            float phase = Clamp01(elapsed / scareRuntime_.fleshFlickerDuration);
            float envelope = SmoothStep(0.0f, 0.045f, elapsed) * (1.0f - SmoothStep(scareRuntime_.fleshFlickerDuration - 0.10f, scareRuntime_.fleshFlickerDuration, elapsed));
            float strobe = (std::sin(phase * kPi * 4.0f) > 0.0f) ? 1.0f : 0.0f;
            fleshAmount = envelope * strobe * settingsRuntime_.live.fleshFlickerIntensity;
        }
        if (settingsRuntime_.live.fleshAlwaysOn) fleshAmount = std::max(fleshAmount, settingsRuntime_.live.fleshFlickerIntensity);
        cb.horror0 = {
            Clamp01(fleshAmount),
            std::clamp(settingsRuntime_.live.bloodWetness, 0.0f, 3.0f),
            std::clamp(settingsRuntime_.live.fleshWetness, 0.0f, 4.0f),
            std::clamp(settingsRuntime_.live.fleshParallaxScale, 0.0f, 0.50f)
        };
