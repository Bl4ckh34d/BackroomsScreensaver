        if (panicActive) {
            cameraRuntime_.stopTimer = 0.0f;
            cameraRuntime_.headScanTimer = 0.0f;
            cameraRuntime_.headScanDuration = 0.0f;
            cameraRuntime_.lookBack = false;
            cameraRuntime_.junctionScanActive = false;
            viewRuntime_.propLookTimer = 0.0f;
            cameraRuntime_.branchLookTimer = 0.0f;
            cameraRuntime_.roomSurveyTimer = 0.0f;
            cameraRuntime_.branchLookPaused = false;

            viewRuntime_.chaseLookBackCooldown = std::max(0.0f, viewRuntime_.chaseLookBackCooldown - dt);
            viewRuntime_.chaseLookBackTimer = std::max(0.0f, viewRuntime_.chaseLookBackTimer - dt);
            if (threat && viewRuntime_.chaseLookBackTimer <= 0.0f && viewRuntime_.chaseLookBackCooldown <= 0.0f && MonsterDistance() > 1.55f) {
                float urgency = Clamp01((7.8f - MonsterDistance()) / 6.4f);
                float elapsedPressure = Clamp01(viewRuntime_.secondsSinceLookBack / 9.0f);
                float chance = dt * (0.22f + urgency * 1.55f + elapsedPressure * 0.80f);
                if (RandRange(0.0f, 1.0f) < chance) {
                    BeginChaseLookBack(urgency);
                }
            }
            if (threat) UpdateChaseLookBackTarget(dt);

            viewRuntime_.stumbleTimer = std::max(0.0f, viewRuntime_.stumbleTimer - dt);
            if (threat && viewRuntime_.stumbleTimer <= 0.0f && viewRuntime_.chaseLookBackTimer <= 0.0f && RandRange(0.0f, 1.0f) < dt * 0.14f) {
                viewRuntime_.stumbleDuration = RandRange(0.18f, 0.34f);
                viewRuntime_.stumbleTimer = viewRuntime_.stumbleDuration;
                viewRuntime_.stumbleYawOffset = RandRange(-0.20f, 0.20f);
            }
        } else if (roomSurveySpaceNow && !bloodFocusActive && !ventReactionActive && cameraRuntime_.stopTimer <= 0.0f &&
            viewRuntime_.propLookTimer <= 0.0f && cameraRuntime_.branchLookTimer <= 0.0f && cameraRuntime_.roomSurveyTimer <= 0.0f &&
            timeRuntime_.time > cameraRuntime_.nextLookBackTime) {
            float elapsedPressure = Clamp01(viewRuntime_.secondsSinceLookBack / std::max(2.0f, settingsRuntime_.live.lookBackMaxSeconds));
            float proximityPressure = Clamp01((10.0f - MonsterDistance()) / 8.0f) * 0.35f;
            float chance = dt * (0.08f + elapsedPressure * 0.62f + proximityPressure);
            if (RandRange(0.0f, 1.0f) < chance) {
                cameraRuntime_.stopTimer = RandRange(1.05f, 1.85f);
                cameraRuntime_.headScanDuration = cameraRuntime_.stopTimer;
                cameraRuntime_.headScanTimer = cameraRuntime_.stopTimer;
                cameraRuntime_.headScanCenter = gameWorld_.player.bodyYaw + RandRange(-0.16f, 0.16f);
                cameraRuntime_.lookBack = true;
                cameraRuntime_.branchLookTimer = 0.0f;
                cameraRuntime_.roomSurveyTimer = 0.0f;
                cameraRuntime_.nextLookBackTime = timeRuntime_.time + RandRange(settingsRuntime_.live.lookBackMinSeconds, settingsRuntime_.live.lookBackMaxSeconds);
                viewRuntime_.secondsSinceLookBack = 0.0f;
            }
        }
