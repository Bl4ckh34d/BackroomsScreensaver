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
