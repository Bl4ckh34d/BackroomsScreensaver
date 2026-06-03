        float gentleScale = (1.0f + viewRuntime_.dangerLevel * 1.10f + activeAgitation * 1.05f + dreadPressure * 0.95f) * settingsRuntime_.live.flashlightSwayAmount;
        float gentleYaw =
            std::sin(timeRuntime_.time * 0.31f + std::sin(timeRuntime_.time * 0.13f) * 1.6f) * 0.026f * gentleScale +
            std::sin(timeRuntime_.time * 1.18f + std::sin(timeRuntime_.time * 0.41f) * 0.7f) * 0.044f * gentleScale +
            std::sin(timeRuntime_.time * 2.85f + 1.7f) * 0.020f * gentleScale;
        float gentlePitch =
            std::sin(timeRuntime_.time * 0.27f + 1.4f) * 0.014f * gentleScale +
            std::sin(timeRuntime_.time * 1.02f + 2.4f) * 0.022f * gentleScale +
            std::sin(timeRuntime_.time * 2.32f) * 0.011f * gentleScale;

        float tremorScale = (0.0035f + activeAgitation * 0.0075f + dreadPressure * 0.0105f) * settingsRuntime_.live.flashlightSwayAmount;
        float tremorYaw =
            std::sin(timeRuntime_.time * 12.7f + std::sin(timeRuntime_.time * 3.9f) * 1.4f) * 0.70f +
            std::sin(timeRuntime_.time * 28.9f + 2.1f) * 0.24f +
            std::sin(timeRuntime_.time * 61.0f + 0.7f) * 0.09f;
        float tremorPitch =
            std::sin(timeRuntime_.time * 10.6f + 1.8f) * 0.58f +
            std::sin(timeRuntime_.time * 24.4f + std::sin(timeRuntime_.time * 5.5f)) * 0.28f +
            std::sin(timeRuntime_.time * 57.0f + 2.6f) * 0.08f;
        gentleYaw += tremorYaw * tremorScale;
        gentlePitch += tremorPitch * tremorScale * 0.72f;

        float stepRun = Clamp01(world.playerRunIntensity * 0.78f + world.playerRunEffort * 0.62f);
        if (stepRun > 0.001f) {
            float stepWave = std::sin(world.playerStepPhase);
            float crossWave = std::sin(world.playerStepPhase * 0.5f + 0.65f);
            float runSwing = settingsRuntime_.live.flashlightSwayAmount * stepRun;
            gentleYaw += (stepWave * 0.040f + crossWave * 0.024f) * runSwing;
            gentlePitch += (-stepWave * 0.028f + std::abs(crossWave) * 0.018f) * runSwing;
        }

        if (viewRuntime_.panicFlashlightTimer > 0.0f && viewRuntime_.panicFlashlightDuration > 0.001f) {
            float t = 1.0f - viewRuntime_.panicFlashlightTimer / viewRuntime_.panicFlashlightDuration;
            float envelope = (1.0f - SmoothStep(0.48f, 1.0f, t)) * SmoothStep(0.0f, 0.10f, t);
            float swing = std::sin(t * kPi * 3.35f);
            float snap = std::sin(t * kPi * 7.10f) * 0.26f;
            float panic = envelope * (0.78f + world.monsterChasePanic * 0.42f) * settingsRuntime_.live.flashlightPanicDartAmount;
            gentleYaw += (swing * 0.080f + snap * 0.034f) * panic;
            gentlePitch += (swing * 0.052f - envelope * 0.030f) * panic;
        }

