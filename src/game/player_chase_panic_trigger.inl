
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        float range = std::max(0.1f, MonsterSightDistance());
        float proximity = Clamp01((range - monsterDist) / range);
        bool chaseAlreadyCommitted = gameWorld_.MonsterChaseCommitted();
        if (threat && !viewRuntime_.threatVisibleLast) {
            if (!chaseAlreadyCommitted) {
                gameWorld_.BeginMonsterRecognition(RandRange(0.20f, 0.80f));
                viewRuntime_.panicFlashlightDuration = RandRange(1.10f, 1.55f);
                viewRuntime_.panicFlashlightTimer = viewRuntime_.panicFlashlightDuration;
                viewRuntime_.flashlightAgitation = std::max(viewRuntime_.flashlightAgitation, 1.0f);
                cameraRuntime_.threatRepath = 0.0f;
                cameraRuntime_.path.clear();
                cameraRuntime_.pathIndex = 0;
                cameraRuntime_.turnLookBlend = 0.0f;
                cameraRuntime_.turnLookYaw = world.playerYaw;
                gameWorld_.SetPlayerSmoothedMoveSpeed(0.0f);
            } else {
                gameWorld_.CommitMonsterChaseRecognition();
                cameraRuntime_.threatRepath = 0.0f;
                viewRuntime_.flashlightAgitation = std::max(viewRuntime_.flashlightAgitation, 0.74f);
            }
        }
