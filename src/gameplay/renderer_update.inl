    void UpdateSimulation(float dt) {
        scareRuntime_.visionFlashTimer = std::max(0.0f, scareRuntime_.visionFlashTimer - std::max(0.0f, dt));

        if (gameWorld_.deathActive) {
            UpdateDeath(dt);
            UpdateDreadMeterDisplay(dt);
            UpdateFlashlightAim(dt);
            UpdateAirParticles(dt);
            UpdateAirParticleFocus(dt);
            return;
        }

        PlayerFlashlightInputResult flashlightInput = PlayerController::UpdateFlashlightInput(
            gameWorld_.player,
            sessionRuntime_.IsPlayableGame(),
            sessionRuntime_.input.flashlight);
        if (flashlightInput.toggled) {
            gameWorld_.QueueAudioEvent(GameAudioEvent::OneShotWithPlayerNoise(
                GameSound::FlashlightStutter,
                AudioBus::Effects,
                gameWorld_.player.position,
                0.41f,
                false,
                FlashlightClickHearingRadius(),
                0.62f).WithCategory(GameAudioEventCategory::PlayerInteraction));
        }

        if (sessionRuntime_.IsPlayableGame() && gameWorld_.PlayableScoreScreenActive()) {
            if (PlayerController::ConsumeInteractPress(gameWorld_.player, sessionRuntime_.input.interact)) {
                ContinueAfterScoreScreen();
            }
            UpdateFlashlightAim(dt);
            UpdateAirParticles(dt);
            UpdateAirParticleFocus(dt);
            return;
        }

        viewRuntime_.fadeInTimer = std::max(0.0f, viewRuntime_.fadeInTimer - dt);
        if (sessionRuntime_.IsMainMenu()) {
            UpdateMainMenuScene(dt);
            return;
        }
        if (monsterPreview_.active) {
            UpdateMonsterHeadAnimation(dt, false);
            SetMonsterPreviewCamera(timeRuntime_.time);
            UpdateFlashlightAim(dt);
            UpdateDreadMeterDisplay(dt);
            return;
        }
        if (gEffectDebugViewer) {
            ApplyDebugSliceCamera();
            UpdateDebugSliceLoop(dt);
            UpdateDebugMonsterWalk(dt);
            if (gDebugSliceEffect == DebugSliceEffect::AirVents) {
                UpdateSteamAndDrops(dt);
            } else if (gDebugSliceEffect == DebugSliceEffect::BrokenLamps) {
                UpdateSparks(dt);
            }
            UpdateFlashlightAim(dt);
            UpdateDreadMeterDisplay(dt);
            return;
        }
        if (settingsRuntime_.live.bloodStudyView) {
            ApplyBloodStudyCamera();
            scareRuntime_.bloodWorldActivationTime = timeRuntime_.time - 46.0f;
            UpdateSparks(dt);
            UpdateSteamAndDrops(dt);
            UpdateFlashlightAim(dt);
            UpdateDreadMeterDisplay(dt);
            UpdateAirParticles(dt);
            UpdateAirParticleFocus(dt);
            return;
        }
        if (benchmarkRuntime_.active || BenchmarkDemoEnabled()) {
            benchmarkRuntime_.active = true;
            benchmarkRuntime_.timer += dt;
            UpdateScareEvents(dt);
            UpdateSparks(dt);
            UpdateSteamAndDrops(dt);
            UpdateBrokenRuntimeLampSparks(dt, 3.0f, 9.5f, 0.16f);
            if (MonsterActiveForCurrentMode()) {
                UpdateMonsterHeadAnimation(dt, false);
            }
            viewRuntime_.dangerLevel = 0.40f + std::sin(timeRuntime_.time * 0.73f) * 0.12f;
            viewRuntime_.dreadLevel = 0.45f + std::cos(timeRuntime_.time * 0.41f) * 0.10f;
            UpdateDreadMeterDisplay(dt);
            ApplyBenchmarkDemoCamera(benchmarkRuntime_.timer);
            UpdateFlashlightAim(dt);
            UpdateAirParticles(dt);
            UpdateAirParticleFocus(dt);
            float duration = BenchmarkDemoDurationSeconds();
            if (duration > 0.0f && benchmarkRuntime_.timer >= duration && hostRuntime_.hwnd) {
                PostMessageW(hostRuntime_.hwnd, WM_CLOSE, 0, 0);
            }
            return;
        }
        UpdateScareEvents(dt);
        UpdateSparks(dt);
        UpdateSteamAndDrops(dt);
        if (gameWorld_.exitTransitionActive) {
            UpdateExitTransition(dt);
            UpdateFlashlightAim(dt);
            UpdateAirParticles(dt);
            UpdateAirParticleFocus(dt);
            return;
        }

        if (VisibleInFront(gameWorld_.maze.exit)) viewRuntime_.exitSpotted = true;
        bool monsterActive = MonsterActiveForCurrentMode();
        float monsterDist = monsterActive ? MonsterDistance() : std::numeric_limits<float>::max();
        if (monsterActive) {
            MonsterUpdateInput monsterInput = BuildMonsterUpdateInput(dt);
            MonsterUpdateOutput monsterOutput = UpdateMonster(monsterInput);
            UpdateMonsterLampDamage(dt);
            monsterDist = monsterOutput.distanceToPlayer;
            if (monsterOutput.HasGameplayEvent(MonsterGameplayEventKind::KillPlayer)) {
                BeginDeath();
                UpdateDeath(dt);
                UpdateFlashlightAim(dt);
                UpdateAirParticles(dt);
                UpdateAirParticleFocus(dt);
                return;
            }
        } else {
            gameWorld_.monster.ClearPursuitState();
            ResetMonsterPresentationState(false, false);
        }

        bool threat = monsterActive && IsThreatVisible();
        if (!monsterActive || MonsterIgnoresPlayer()) {
            threat = false;
            gameWorld_.monster.ClearChaseCommitment();
        }
        UpdateChasePanic(dt, threat, monsterDist);
        UpdateBrokenRuntimeLampSparks(dt, 3.0f, 9.5f, 0.12f);
        bool panicActive = threat || ChasePanicActive();
        float dangerTarget = threat ? Clamp01((8.0f - monsterDist) / 6.6f) : gameWorld_.monster.chasePanic * 0.38f;
        viewRuntime_.dangerLevel += (dangerTarget - viewRuntime_.dangerLevel) * std::min(1.0f, dt * (dangerTarget > viewRuntime_.dangerLevel ? 2.2f : 0.85f));
        UpdateDread(dt, threat, monsterDist);
        UpdateMonsterSightDread(dt, threat, monsterDist);
        UpdateBloodDread(dt);
        UpdateMonsterProximityBlood(dt);
        UpdateDreadMeterDisplay(dt);

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
    }
