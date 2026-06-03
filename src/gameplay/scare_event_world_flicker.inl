        bool fleshWorldEnabled = settingsRuntime_.live.fleshFlicker;
        bool bloodWorldEnabled = settingsRuntime_.live.bloodWorldFlicker && settingsRuntime_.live.bloodWorldCoverage > 0.001f;
        if (!fleshWorldEnabled) {
            scareRuntime_.fleshFlickerTimer = 0.0f;
        }
        if (!bloodWorldEnabled) {
            scareRuntime_.bloodWorldFlickerTimer = 0.0f;
        }
        if (!fleshWorldEnabled && !bloodWorldEnabled) {
            scareRuntime_.fleshFlickerCooldown = 1000000.0f;
            scareRuntime_.bloodWorldFlickerCooldown = 1000000.0f;
        } else {
            scareRuntime_.bloodWorldFlickerCooldown = std::max(0.0f, scareRuntime_.bloodWorldFlickerCooldown - dt);
            scareRuntime_.fleshFlickerCooldown = scareRuntime_.bloodWorldFlickerCooldown;
            if (scareRuntime_.bloodWorldFlickerCooldown <= 0.0f && scareRuntime_.fleshFlickerTimer <= 0.0f &&
                scareRuntime_.bloodWorldFlickerTimer <= 0.0f && scareRuntime_.scareCooldown <= 0.0f) {
                bool bloodAllowed = bloodWorldEnabled && customScareAllowed(3);
                bool fleshAllowed = fleshWorldEnabled && customScareAllowed(4);
                bool triggerBloodWorld = bloodAllowed && (!fleshAllowed || RandRange(0.0f, 1.0f) < 0.50f);
                if (!triggerBloodWorld) {
                    if (!fleshAllowed) {
                        scareRuntime_.bloodWorldFlickerCooldown = RandRange(1.0f, 3.0f);
                        scareRuntime_.fleshFlickerCooldown = scareRuntime_.bloodWorldFlickerCooldown;
                    } else {
                        scareRuntime_.fleshFlickerDuration = settingsRuntime_.live.fleshFlickerDuration;
                        scareRuntime_.fleshFlickerTimer = scareRuntime_.fleshFlickerDuration;
                        TriggerVisionFlashJumpscare(false);
                        scareRuntime_.scareCooldown = std::max(scareRuntime_.scareCooldown, scareRuntime_.fleshFlickerDuration + RandRange(8.0f, 18.0f) * scareScale);
                        viewRuntime_.flashlightAgitation = std::max(viewRuntime_.flashlightAgitation, 0.62f);
                        AddDread(settingsRuntime_.live.dreadFleshGain);
                    }
                } else {
                    scareRuntime_.bloodWorldFlickerDuration = settingsRuntime_.live.bloodWorldFlickerDuration;
                    scareRuntime_.bloodWorldFlickerTimer = scareRuntime_.bloodWorldFlickerDuration;
                    TriggerVisionFlashJumpscare(true);
                    scareRuntime_.bloodWorldActivationTime = timeRuntime_.time;
                    scareRuntime_.scareCooldown = std::max(scareRuntime_.scareCooldown, scareRuntime_.bloodWorldFlickerDuration + RandRange(9.0f, 20.0f) * scareScale);
                    viewRuntime_.flashlightAgitation = std::max(viewRuntime_.flashlightAgitation, 0.72f);
                    AddDread(std::max(settingsRuntime_.live.dreadJumpscareGain * 0.90f, 0.30f));
                }
                float worldMinSeconds = std::min(
                    fleshWorldEnabled ? settingsRuntime_.live.fleshFlickerMinSeconds : settingsRuntime_.live.bloodWorldFlickerMinSeconds,
                    bloodWorldEnabled ? settingsRuntime_.live.bloodWorldFlickerMinSeconds : settingsRuntime_.live.fleshFlickerMinSeconds);
                float worldMaxSeconds = std::max(
                    fleshWorldEnabled ? settingsRuntime_.live.fleshFlickerMaxSeconds : settingsRuntime_.live.bloodWorldFlickerMaxSeconds,
                    bloodWorldEnabled ? settingsRuntime_.live.bloodWorldFlickerMaxSeconds : settingsRuntime_.live.fleshFlickerMaxSeconds);
                scareRuntime_.bloodWorldFlickerCooldown = RandRange(worldMinSeconds, std::max(worldMinSeconds, worldMaxSeconds));
                scareRuntime_.fleshFlickerCooldown = scareRuntime_.bloodWorldFlickerCooldown;
            }
        }
        if (gameWorld_.deathActive || gameWorld_.exitTransitionActive) return;
