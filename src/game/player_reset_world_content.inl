        exitDoorPresentation_.angle = 0.0f;
        gameWorld_.GenerateCollectiblePagesForCurrentLevel(
            IsPlayableSimulationMode(sessionRuntime_.mode),
            settingsRuntime_.live.wallHeightMeters,
            sessionRuntime_.rng);
        gameWorld_.TryGenerateSavePoint(
            sessionRuntime_.mode == RendererRuntimeMode::PlayableGame,
            settingsRuntime_.live.saveItemLevelChance,
            sessionRuntime_.rng);
        ResetMonsterPresentationState(true, true);
        if (MonsterActiveForCurrentMode()) {
            gameWorld_.ResetMonsterForSession(RandRange(0.8f, 2.4f));
            PrimeMonsterTrail(gameWorld_.MazeTileMinimum() * 0.075f);
        } else {
            gameWorld_.monster.ClearPursuitState();
        }
        monsterPresentation_.headBobPhase = RandRange(0.0f, kPi * 2.0f);
        monsterPresentation_.headScanPhase = RandRange(0.0f, kPi * 2.0f);
        monsterPresentation_.headYawOffset = 0.0f;
        monsterPresentation_.headPitchOffset = 0.0f;
