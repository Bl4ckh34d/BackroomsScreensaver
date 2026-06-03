        if (threat && !MonsterSightingFreezeActive()) {
            cameraRuntime_.stopTimer = 0.0f;
            cameraRuntime_.headScanTimer = 0.0f;
            cameraRuntime_.headScanDuration = 0.0f;
            cameraRuntime_.lookBack = false;
            cameraRuntime_.junctionScanActive = false;
            cameraRuntime_.branchLookTimer = 0.0f;
            cameraRuntime_.roomSurveyTimer = 0.0f;
            viewRuntime_.propLookTimer = 0.0f;
            scareRuntime_.bloodFocusTimer = 0.0f;
            viewRuntime_.ventReactionTimer = 0.0f;
            cameraRuntime_.threatRepath -= dt;
            if (cameraRuntime_.threatRepath <= 0.0f || cameraRuntime_.pathIndex >= cameraRuntime_.path.size()) {
                ChoosePath(true);
                bool runningToExit = !cameraRuntime_.path.empty() && cameraRuntime_.path.back() == gameWorld_.maze.exit;
                bool committedEscape = FirstThreatLineBreakIndex(cameraRuntime_.path, MonsterTile(), 7) >= 0 || FirstBranchIndex(cameraRuntime_.path, 6) >= 0;
                cameraRuntime_.threatRepath = runningToExit ? RandRange(3.4f, 5.4f) : (committedEscape ? RandRange(2.6f, 4.2f) : RandRange(1.2f, 2.2f));
            }
        } else if (threat) {
            cameraRuntime_.threatRepath = 0.0f;
            cameraRuntime_.path.clear();
            cameraRuntime_.pathIndex = 0;
        } else if (panicActive) {
            cameraRuntime_.threatRepath = 0.0f;
            viewRuntime_.chaseLookBackTimer = std::max(0.0f, viewRuntime_.chaseLookBackTimer - dt * 1.20f);
            viewRuntime_.chaseLookBackCooldown = std::max(0.0f, viewRuntime_.chaseLookBackCooldown - dt);
            viewRuntime_.stumbleTimer = std::max(0.0f, viewRuntime_.stumbleTimer - dt * 1.10f);
        } else {
            cameraRuntime_.threatRepath = 0.0f;
            viewRuntime_.chaseLookBackTimer = 0.0f;
            viewRuntime_.chaseLookBackCooldown = std::max(0.0f, viewRuntime_.chaseLookBackCooldown - dt);
            viewRuntime_.stumbleTimer = std::max(0.0f, viewRuntime_.stumbleTimer - dt * 2.0f);
        }
        if (sessionRuntime_.UsesManualInput()) {
            UpdateManualPlayer(dt);
        } else {
            UpdatePathFollower(dt);
        }
        UpdateFlashlightAim(dt);
        UpdateAirParticles(dt);
        UpdateAirParticleFocus(dt);
