        float targetBonus = static_cast<float>(std::max(0, cameraRuntime_.roomSurveyYawCount - 1)) * (pauseFirst ? 0.32f : 0.20f);
        cameraRuntime_.roomSurveyDuration = (pauseFirst ? RandRange(1.95f, 2.70f) : RandRange(1.18f, 1.85f)) + targetBonus;
        cameraRuntime_.roomSurveyTimer = cameraRuntime_.roomSurveyDuration;
        cameraRuntime_.roomSurveyCooldown = RandRange(4.5f, 9.5f);
        cameraRuntime_.branchLookTimer = 0.0f;
        cameraRuntime_.branchLookPaused = false;
        if (pauseFirst) {
            cameraRuntime_.stopTimer = std::max(cameraRuntime_.stopTimer, cameraRuntime_.roomSurveyDuration * RandRange(0.66f, 0.86f));
            cameraRuntime_.headScanTimer = 0.0f;
            cameraRuntime_.headScanDuration = 0.0f;
            cameraRuntime_.junctionScanActive = false;
            cameraRuntime_.lookBack = false;
            viewRuntime_.propLookTimer = 0.0f;
        }
