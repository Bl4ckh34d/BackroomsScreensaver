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
