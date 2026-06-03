// Dread, chase-panic, and monster-sighting pressure helpers. 
// Included inside Renderer's private section from player_monster_pressure.inl.

    void AddDread(float amount) {
        if (!settingsRuntime_.live.dreadEnabled || amount <= 0.0f) return;
        viewRuntime_.dreadLevel = Clamp01(viewRuntime_.dreadLevel + amount);
        viewRuntime_.flashlightAgitation = std::max(viewRuntime_.flashlightAgitation, std::min(1.0f, amount * 2.2f));
    }

    void UpdateDreadMeterDisplay(float dt) {
        if (!settingsRuntime_.live.dreadEnabled) {
            viewRuntime_.dreadMeterLevel = 0.0f;
            return;
        }
        float response = viewRuntime_.dreadLevel > viewRuntime_.dreadMeterLevel ? 18.0f : 5.5f;
        viewRuntime_.dreadMeterLevel += (viewRuntime_.dreadLevel - viewRuntime_.dreadMeterLevel) * std::min(1.0f, std::max(0.0f, dt) * response);
    }

    void UpdateChasePanic(float dt, bool threat, float monsterDist) {
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
        if (threat) {
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
            gameWorld_.AdvanceMonsterRecognition(dt);
            if (gameWorld_.MonsterRecognitionExpired()) {
                if (gameWorld_.MonsterRecognitionPending() && !MonsterCuriosityActive()) {
                    gameWorld_.BeginMonsterCuriosity(RandRange(0.98f, 1.26f));
                }
                if (MonsterCuriosityActive()) {
                    gameWorld_.AdvanceMonsterCuriosity(dt);
                }
                if (!MonsterCuriosityActive() && !gameWorld_.MonsterRecognizedForChase()) {
                    viewRuntime_.monsterRunLaunchMeters = 0.0f;
                    viewRuntime_.monsterRunLaunchActive = true;
                    gameWorld_.RaisePlayerRunPressure(0.86f, 0.94f, settingsRuntime_.live.runSpeed * 0.42f);
                    gameWorld_.CommitMonsterChaseRecognition();
                }
                if (!MonsterCuriosityActive()) gameWorld_.CancelMonsterRecognition();
            }
            float hold = Lerp(2.4f, 5.2f, SmoothStep(0.12f, 0.82f, proximity));
            gameWorld_.HoldMonsterChaseMemory(hold);
        } else {
            if (MonsterCuriosityActive()) {
                gameWorld_.AdvanceMonsterCuriosity(dt);
                if (!MonsterCuriosityActive() && gameWorld_.MonsterChaseMemoryTimer() > 0.0f && !gameWorld_.MonsterRecognizedForChase()) {
                    viewRuntime_.monsterRunLaunchMeters = 0.0f;
                    viewRuntime_.monsterRunLaunchActive = true;
                    gameWorld_.CommitMonsterChaseRecognition();
                }
            }
            gameWorld_.CancelMonsterRecognition();
            gameWorld_.DecayMonsterChaseMemory(dt);
        }

        float target = 0.0f;
        if (threat) {
            target = std::max(0.54f, SmoothStep(0.04f, 0.82f, proximity));
        } else if (gameWorld_.MonsterChaseMemoryTimer() > 0.0f) {
            target = Clamp01(gameWorld_.MonsterChaseMemoryTimer() / 4.8f) * 0.72f;
        }

        float response = target > gameWorld_.MonsterChasePanic()
            ? (threat ? 1.85f : 0.75f)
            : (gameWorld_.MonsterChaseMemoryTimer() > 0.0f ? 0.45f : 0.80f);
        gameWorld_.UpdateMonsterChasePanic(target, response, dt);
        if (!threat && gameWorld_.MonsterChaseMemoryTimer() <= 0.0f && gameWorld_.MonsterChasePanic() <= 0.02f) {
            gameWorld_.ReleaseMonsterChaseRecognition();
            viewRuntime_.monsterRunLaunchActive = false;
            viewRuntime_.monsterRunLaunchMeters = 3.0f;
            gameWorld_.ClearMonsterCuriosity();
        }
        viewRuntime_.threatVisibleLast = threat;
    }

    bool ChasePanicActive() const {
        return gameWorld_.MonsterChasePanicActive();
    }

    bool MonsterSightingFreezeActive() const {
        return gameWorld_.MonsterRecognitionFreezeActive();
    }

    void UpdateDread(float dt, bool threat, float monsterDist) {
        if (!settingsRuntime_.live.dreadEnabled) {
            viewRuntime_.dreadLevel = 0.0f;
            viewRuntime_.dreadMeterLevel = 0.0f;
            viewRuntime_.monsterSpottedLast = false;
            viewRuntime_.monsterSightDreadCooldown = 0.0f;
            return;
        }
        float decay = settingsRuntime_.live.dreadDecayPerSecond * dt * (threat ? 0.20f : 1.0f);
        viewRuntime_.dreadLevel = std::max(0.0f, viewRuntime_.dreadLevel - decay);

        float proximity = Clamp01((settingsRuntime_.live.dreadMonsterDistance - monsterDist) / std::max(0.1f, settingsRuntime_.live.dreadMonsterDistance));
        if (proximity > 0.0f) {
            float visibleWeight = threat ? 1.0f : 0.32f;
            float gain = std::pow(proximity, 1.45f) * settingsRuntime_.live.dreadMonsterGainPerSecond * visibleWeight * dt;
            viewRuntime_.dreadLevel = Clamp01(viewRuntime_.dreadLevel + gain);
            if (threat) {
                viewRuntime_.dreadLevel = std::max(viewRuntime_.dreadLevel, proximity * 0.58f);
            }
        }
    }

    void UpdateMonsterSightDread(float dt, bool threat, float monsterDist) {
        if (!settingsRuntime_.live.dreadEnabled) return;
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        viewRuntime_.monsterSightDreadCooldown = std::max(0.0f, viewRuntime_.monsterSightDreadCooldown - dt);
        bool spotted = threat && PlayerLooksAt({world.monsterPosition.x, 1.18f, world.monsterPosition.z}, MonsterSightDistance(), 0.38f);
        if (spotted && !viewRuntime_.monsterSpottedLast && viewRuntime_.monsterSightDreadCooldown <= 0.0f) {
            float proximity = Clamp01((settingsRuntime_.live.dreadMonsterDistance - monsterDist) / std::max(0.1f, settingsRuntime_.live.dreadMonsterDistance));
            float spike = std::max(settingsRuntime_.live.dreadJumpscareGain * 1.08f, 0.36f + proximity * 0.24f);
            AddDread(spike);
            viewRuntime_.flashlightAgitation = std::max(viewRuntime_.flashlightAgitation, 0.88f);
            viewRuntime_.monsterSightDreadCooldown = RandRange(5.5f, 8.5f);
        }
        viewRuntime_.monsterSpottedLast = spotted;
    }

    float DreadPressure() const {
        if (!settingsRuntime_.live.dreadEnabled) return 0.0f;
        float proximity = Clamp01((settingsRuntime_.live.dreadMonsterDistance - MonsterDistance()) / std::max(0.1f, settingsRuntime_.live.dreadMonsterDistance));
        return Clamp01(std::max(viewRuntime_.dreadLevel * 0.86f, proximity * 0.74f));
    }

    float DreadFlashlightMultiplier() const {
        if (!settingsRuntime_.live.dreadEnabled) return 1.0f;
        float monsterRange = std::max(0.1f, settingsRuntime_.live.dreadMonsterDistance);
        float monsterProximity = Clamp01((monsterRange - MonsterDistance()) / monsterRange);
        float pressure = Clamp01(SmoothStep(0.10f, 1.0f, monsterProximity) * settingsRuntime_.live.dreadFlashlightFlicker);
        if (pressure <= 0.02f) return 1.0f;
        float waves =
            std::sin(timeRuntime_.time * 19.7f + std::sin(timeRuntime_.time * 3.1f) * 0.8f) +
            std::sin(timeRuntime_.time * 43.3f + 1.8f) * 0.58f +
            std::sin(timeRuntime_.time * 91.9f + 0.4f) * 0.28f;
        float gate = waves > (1.06f - pressure * 1.18f) ? 1.0f : 0.0f;
        float flutter = 0.5f + 0.5f * std::sin(timeRuntime_.time * (11.0f + pressure * 29.0f) + std::sin(timeRuntime_.time * 5.7f) * 1.4f);
        float drop = pressure * (0.05f + flutter * 0.12f + gate * (0.24f + pressure * 0.42f));
        return std::clamp(1.0f - drop, 0.18f, 1.05f);
    }
