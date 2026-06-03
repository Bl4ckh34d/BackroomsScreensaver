        float activeAgitation = std::max(viewRuntime_.flashlightAgitation, dreadPressure * 0.68f);
        auto mergeFlashlightHold = [&](float yawOffset, float pitchOffset, float carry) {
            constexpr float kHoldYawLimit = 0.24f;
            constexpr float kHoldPitchLimit = 0.15f;
            viewRuntime_.flashlightHoldYaw = std::clamp(viewRuntime_.flashlightHoldYaw * carry + yawOffset, -kHoldYawLimit, kHoldYawLimit);
            viewRuntime_.flashlightHoldPitch = std::clamp(viewRuntime_.flashlightHoldPitch * carry + pitchOffset, -kHoldPitchLimit, kHoldPitchLimit);
        };

        viewRuntime_.flashlightSnapCooldown = std::max(0.0f, viewRuntime_.flashlightSnapCooldown - dt);
        viewRuntime_.flashlightSnapTimer = std::max(0.0f, viewRuntime_.flashlightSnapTimer - dt);
        if (viewRuntime_.flashlightSnapTimer <= 0.0f && viewRuntime_.flashlightSnapCooldown <= 0.0f) {
            float pressure = Clamp01(activeAgitation * 0.72f + dreadPressure * 0.48f + fastTurn * 0.40f);
            viewRuntime_.flashlightSnapSharp = RandRange(0.0f, 1.0f) < (0.22f + pressure * 0.56f);
            viewRuntime_.flashlightSnapDuration = viewRuntime_.flashlightSnapSharp
                ? RandRange(0.055f, 0.14f)
                : RandRange(0.34f, 1.18f);
            viewRuntime_.flashlightSnapTimer = viewRuntime_.flashlightSnapDuration;
            float sharpBoost = viewRuntime_.flashlightSnapSharp ? 1.55f : 0.62f;
            float amount = (0.45f + pressure * 1.10f) * settingsRuntime_.live.flashlightSwayAmount * sharpBoost;
            viewRuntime_.flashlightSnapYaw = RandRange(-0.075f, 0.075f) * amount;
            viewRuntime_.flashlightSnapPitch = RandRange(-0.040f, 0.050f) * amount;
            mergeFlashlightHold(viewRuntime_.flashlightSnapYaw, viewRuntime_.flashlightSnapPitch, viewRuntime_.flashlightSnapSharp ? 0.42f : 0.64f);
            viewRuntime_.flashlightSnapCooldown = viewRuntime_.flashlightSnapSharp
                ? RandRange(0.34f, 0.95f) * (1.0f - pressure * 0.28f)
                : RandRange(0.80f, 2.60f) * (1.0f - pressure * 0.22f);
            viewRuntime_.flashlightSnapCooldown = std::max(0.18f, viewRuntime_.flashlightSnapCooldown);
        }

        if (activeAgitation > 0.14f && viewRuntime_.flashlightDartTimer <= 0.0f && viewRuntime_.flashlightDartCooldown <= 0.0f) {
            viewRuntime_.flashlightDartDuration = RandRange(0.105f, 0.24f);
            viewRuntime_.flashlightDartTimer = viewRuntime_.flashlightDartDuration;
            float amount = SmoothStep(0.10f, 1.0f, activeAgitation);
            float dreadJitter = 1.0f + dreadPressure * 0.75f;
            viewRuntime_.flashlightDartYaw = RandRange(-0.22f, 0.22f) * amount * dreadJitter * settingsRuntime_.live.flashlightPanicDartAmount;
            viewRuntime_.flashlightDartPitch = RandRange(-0.13f, 0.10f) * amount * dreadJitter * settingsRuntime_.live.flashlightPanicDartAmount;
            mergeFlashlightHold(std::clamp(viewRuntime_.flashlightDartYaw * 0.46f, -0.18f, 0.18f),
                std::clamp(viewRuntime_.flashlightDartPitch * 0.48f, -0.11f, 0.11f), 0.54f);
            float cooldown = RandRange(0.26f, 0.62f) * (1.0f - amount * 0.18f) * (1.0f - dreadPressure * 0.12f);
            viewRuntime_.flashlightDartCooldown = std::max(0.16f * kDartFrequencyDivisor, cooldown * kDartFrequencyDivisor);
        }

