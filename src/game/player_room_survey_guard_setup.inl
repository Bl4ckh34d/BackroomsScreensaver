
        if (!IsRoomSurveySpot(cur) || DreadPressure() > 0.42f || ChasePanicActive() || IsThreatVisible() ||
            (!pauseFirst && cameraRuntime_.roomSurveyCooldown > 0.0f)) {
            return;
        }

        cameraRuntime_.roomSurveyCenter = gameWorld_.player.bodyYaw;
        cameraRuntime_.roomSurveySpan = std::clamp(0.52f + static_cast<float>(gameWorld_.maze.LocalOpenCount(cur, 2)) * 0.030f, 0.62f, 1.18f);
        cameraRuntime_.roomSurveyDirection = RandRange(0.0f, 1.0f) < 0.5f ? -1.0f : 1.0f;
        cameraRuntime_.roomSurveyYawCount = 0;
        cameraRuntime_.roomSurveyPitchCount = 0;
