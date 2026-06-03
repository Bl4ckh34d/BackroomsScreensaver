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
