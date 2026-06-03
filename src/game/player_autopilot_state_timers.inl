        bool monsterActive = MonsterActiveForCurrentMode();
        bool threat = monsterActive && IsThreatVisible();
        bool panicActive = monsterActive && (threat || ChasePanicActive());
        bool sightFreezeActive = threat && MonsterSightingFreezeActive();
        if (panicActive) {
            scareRuntime_.bloodFocusTimer = 0.0f;
            viewRuntime_.ventReactionTimer = 0.0f;
            cameraRuntime_.branchLookTimer = 0.0f;
            cameraRuntime_.roomSurveyTimer = 0.0f;
            cameraRuntime_.branchLookPaused = false;
        } else {
            scareRuntime_.bloodFocusTimer = std::max(0.0f, scareRuntime_.bloodFocusTimer - dt);
            viewRuntime_.ventReactionTimer = std::max(0.0f, viewRuntime_.ventReactionTimer - dt);
            cameraRuntime_.branchLookTimer = std::max(0.0f, cameraRuntime_.branchLookTimer - dt);
            cameraRuntime_.roomSurveyTimer = std::max(0.0f, cameraRuntime_.roomSurveyTimer - dt);
            cameraRuntime_.branchLookCooldown = std::max(0.0f, cameraRuntime_.branchLookCooldown - dt);
            cameraRuntime_.roomSurveyCooldown = std::max(0.0f, cameraRuntime_.roomSurveyCooldown - dt);
            if (cameraRuntime_.branchLookTimer <= 0.0f) cameraRuntime_.branchLookPaused = false;
        }
        bool bloodFocusActive = scareRuntime_.bloodFocusTimer > 0.0f;
        bool ventReactionActive = !panicActive && viewRuntime_.ventReactionTimer > 0.0f;
        bool softStopActive = !panicActive && (cameraRuntime_.stopTimer > 0.0f || bloodFocusActive || ventReactionActive);
        float ventReactionElapsed = 0.0f;
        float ventLookWeight = 0.0f;
        float ventBackAwayWeight = 0.0f;
        if (ventReactionActive) {
            ventReactionElapsed = std::max(0.0f, viewRuntime_.ventReactionDuration - viewRuntime_.ventReactionTimer);
            ventLookWeight = SmoothStep(0.0f, 0.16f, ventReactionElapsed - viewRuntime_.ventReactionLookDelay);
            float backT = (ventReactionElapsed - viewRuntime_.ventReactionLookDelay) / std::max(0.12f, viewRuntime_.ventReactionBackDuration);
            ventBackAwayWeight = SmoothStep(0.0f, 0.18f, backT) * (1.0f - SmoothStep(0.76f, 1.0f, backT));
        }
        bool completedJunctionScan = false;
        bool roomSurveySpaceNow = IsRoomSurveySpot(CameraTile());
        if (cameraRuntime_.stopTimer <= 0.0f && !bloodFocusActive) {
            viewRuntime_.secondsSinceLookBack += dt;
        }
