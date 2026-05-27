    void UpdateSimulation(float dt) {
        if (deathActive_) {
            UpdateDeath(dt);
            UpdateDreadMeterDisplay(dt);
            UpdateFlashlightAim(dt);
            UpdateAirParticles(dt);
            UpdateAirParticleFocus(dt);
            return;
        }

        fadeInTimer_ = std::max(0.0f, fadeInTimer_ - dt);
        if (runtimeMode_ == RendererRuntimeMode::MainMenu) {
            UpdateMainMenuScene(dt);
            return;
        }
        if (monsterPreview_) {
            UpdateMonsterHeadAnimation(dt, false);
            SetMonsterPreviewCamera(time_);
            UpdateFlashlightAim(dt);
            UpdateDreadMeterDisplay(dt);
            return;
        }
        if (gEffectDebugViewer) {
            ApplyDebugSliceCamera();
            UpdateDebugSliceLoop(dt);
            if (gDebugSliceEffect == DebugSliceEffect::AirVents) {
                UpdateSteamAndDrops(dt);
            } else if (gDebugSliceEffect == DebugSliceEffect::BrokenLamps) {
                UpdateSparks(dt);
            }
            UpdateFlashlightAim(dt);
            UpdateDreadMeterDisplay(dt);
            return;
        }
        if (settings_.bloodStudyView) {
            ApplyBloodStudyCamera();
            bloodWorldActivationTime_ = time_ - 46.0f;
            UpdateSparks(dt);
            UpdateSteamAndDrops(dt);
            UpdateFlashlightAim(dt);
            UpdateDreadMeterDisplay(dt);
            UpdateAirParticles(dt);
            UpdateAirParticleFocus(dt);
            return;
        }
        UpdateScareEvents(dt);
        UpdateSparks(dt);
        UpdateSteamAndDrops(dt);
        if (exitTransitionActive_) {
            UpdateExitTransition(dt);
            UpdateFlashlightAim(dt);
            UpdateAirParticles(dt);
            UpdateAirParticleFocus(dt);
            return;
        }

        if (VisibleInFront(maze_.exit)) exitSpotted_ = true;
        UpdateMonster(dt);
        UpdateMonsterLampDamage(dt);
        float monsterDist = MonsterDistance();
        if (monsterDist < settings_.monsterKillDistance && maze_.LineClear(CameraTile(), MonsterTile())) {
            playerHealth_ = 0.0f;
            BeginDeath();
            UpdateDeath(dt);
            UpdateFlashlightAim(dt);
            UpdateAirParticles(dt);
            UpdateAirParticleFocus(dt);
            return;
        }

        bool threat = IsThreatVisible();
        UpdateChasePanic(dt, threat, monsterDist);
        UpdateBrokenRuntimeLampSparks(dt, 3.0f, 9.5f, 0.12f);
        bool panicActive = threat || ChasePanicActive();
        float dangerTarget = threat ? Clamp01((8.0f - monsterDist) / 6.6f) : chasePanic_ * 0.38f;
        dangerLevel_ += (dangerTarget - dangerLevel_) * std::min(1.0f, dt * (dangerTarget > dangerLevel_ ? 2.2f : 0.85f));
        UpdateDread(dt, threat, monsterDist);
        UpdateMonsterSightDread(dt, threat, monsterDist);
        UpdateBloodDread(dt);
        UpdateMonsterProximityBlood(dt);
        UpdateDreadMeterDisplay(dt);

        if (threat && !MonsterSightingFreezeActive()) {
            stopTimer_ = 0.0f;
            headScanTimer_ = 0.0f;
            headScanDuration_ = 0.0f;
            lookBack_ = false;
            junctionScanActive_ = false;
            branchLookTimer_ = 0.0f;
            roomSurveyTimer_ = 0.0f;
            propLookTimer_ = 0.0f;
            bloodFocusTimer_ = 0.0f;
            ventReactionTimer_ = 0.0f;
            threatRepath_ -= dt;
            if (threatRepath_ <= 0.0f || pathIndex_ >= path_.size()) {
                ChoosePath(true);
                bool runningToExit = !path_.empty() && path_.back() == maze_.exit;
                bool committedEscape = FirstThreatLineBreakIndex(path_, MonsterTile(), 7) >= 0 || FirstBranchIndex(path_, 6) >= 0;
                threatRepath_ = runningToExit ? RandRange(3.4f, 5.4f) : (committedEscape ? RandRange(2.6f, 4.2f) : RandRange(1.2f, 2.2f));
            }
        } else if (threat) {
            threatRepath_ = 0.0f;
            path_.clear();
            pathIndex_ = 0;
        } else if (panicActive) {
            threatRepath_ = 0.0f;
            chaseLookBackTimer_ = std::max(0.0f, chaseLookBackTimer_ - dt * 1.20f);
            chaseLookBackCooldown_ = std::max(0.0f, chaseLookBackCooldown_ - dt);
            stumbleTimer_ = std::max(0.0f, stumbleTimer_ - dt * 1.10f);
        } else {
            threatRepath_ = 0.0f;
            chaseLookBackTimer_ = 0.0f;
            chaseLookBackCooldown_ = std::max(0.0f, chaseLookBackCooldown_ - dt);
            stumbleTimer_ = std::max(0.0f, stumbleTimer_ - dt * 2.0f);
        }
        if (runtimeMode_ == RendererRuntimeMode::PlayableGame) {
            UpdateManualPlayer(dt);
        } else {
            UpdatePathFollower(dt);
        }
        UpdateFlashlightAim(dt);
        UpdateAirParticles(dt);
        UpdateAirParticleFocus(dt);
    }
