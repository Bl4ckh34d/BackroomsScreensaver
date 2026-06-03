        float runBob = Clamp01((speed - settingsRuntime_.live.walkSpeed) / std::max(0.1f, settingsRuntime_.live.runSpeed * 1.55f - settingsRuntime_.live.walkSpeed));
        gameWorld_.player.runIntensity += (runBob - gameWorld_.player.runIntensity) * std::min(1.0f, dt * (runBob > gameWorld_.player.runIntensity ? (panicActive ? 7.4f : 4.2f) : 1.15f));
        float effortTarget = Clamp01(gameWorld_.player.runIntensity * (panicActive ? 1.18f : 0.82f) + gameWorld_.monster.chasePanic * 0.46f);
        gameWorld_.player.runEffort += (effortTarget - gameWorld_.player.runEffort) * std::min(1.0f, dt * (effortTarget > gameWorld_.player.runEffort ? (panicActive ? 2.4f : 0.55f) : 0.24f));
        gameWorld_.player.breathPhase += dt * (1.15f + gameWorld_.player.runEffort * 4.8f + gameWorld_.player.runIntensity * 1.45f) * kPi;
        if (gameWorld_.player.breathPhase > kPi * 128.0f) {
            gameWorld_.player.breathPhase = std::fmod(gameWorld_.player.breathPhase, kPi * 2.0f);
        }
