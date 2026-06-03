        effectRuntime_.ventDrops.clear();
        scareRuntime_.bloodScarePoints.clear();
        scareRuntime_.bloodRevealRegions.clear();
        scareRuntime_.activeBloodScareIndex = -1;
        scareRuntime_.bloodScareActiveUntil = 0.0f;
        scareRuntime_.bloodWorldFlickerTimer = 0.0f;
        scareRuntime_.bloodWorldFlickerDuration = settingsRuntime_.live.bloodWorldFlickerDuration;
        scareRuntime_.bloodWorldActivationTime = -1000.0f;
        scareRuntime_.bloodFocusTimer = 0.0f;
        scareRuntime_.bloodFocusDuration = 0.0f;
        scareRuntime_.bloodFocusReactionsTaken = 0;
        scareRuntime_.bloodFocusReactionCooldown = 0.0f;
        scareRuntime_.proximityBloodPulseCooldown = RandRange(4.5f, 9.0f);
        scareRuntime_.bloodFocusTarget = {};
        effectRuntime_.sparkCooldown = settingsRuntime_.live.sparkParticles
            ? RandRange(settingsRuntime_.live.sparkBurstMinSeconds, settingsRuntime_.live.sparkBurstMaxSeconds) * AmbientSparkCooldownScale()
            : 1000000.0f;
        scareRuntime_.scareCooldown = RandRange(7.0f, 15.0f) * ScareCooldownScale();
        scareRuntime_.scareEventTile = CameraTile();
        scareRuntime_.fleshFlickerTimer = 0.0f;
        scareRuntime_.fleshFlickerDuration = settingsRuntime_.live.fleshFlickerDuration;
        scareRuntime_.visionFlashTimer = 0.0f;
        scareRuntime_.visionFlashDuration = 0.16f;
        bool worldFlickersEnabled = settingsRuntime_.live.fleshFlicker ||
            (settingsRuntime_.live.bloodWorldFlicker && settingsRuntime_.live.bloodWorldCoverage > 0.001f);
        if (worldFlickersEnabled) {
            float worldMinSeconds = std::min(settingsRuntime_.live.fleshFlickerMinSeconds, settingsRuntime_.live.bloodWorldFlickerMinSeconds);
            float worldMaxSeconds = std::max(settingsRuntime_.live.fleshFlickerMaxSeconds, settingsRuntime_.live.bloodWorldFlickerMaxSeconds);
            scareRuntime_.bloodWorldFlickerCooldown = RandRange(worldMinSeconds, std::max(worldMinSeconds, worldMaxSeconds));
            scareRuntime_.fleshFlickerCooldown = scareRuntime_.bloodWorldFlickerCooldown;
        } else {
            scareRuntime_.fleshFlickerCooldown = 1000000.0f;
            scareRuntime_.bloodWorldFlickerCooldown = 1000000.0f;
        }
