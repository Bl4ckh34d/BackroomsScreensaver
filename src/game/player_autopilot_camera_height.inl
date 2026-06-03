        float bobAmount = settingsRuntime_.live.headBobAmount * Lerp(0.92f, 2.35f, Clamp01(runBob * 0.72f + gameWorld_.player.runEffort * 0.46f));
        float breathAmount = 0.0035f + gameWorld_.player.runIntensity * 0.028f + gameWorld_.player.runEffort * 0.065f;
        float breathY = std::sin(gameWorld_.player.breathPhase) * breathAmount;
        float walkY = 1.43f + std::abs(std::sin(gameWorld_.player.stepPhase)) * bobAmount +
            std::sin(gameWorld_.player.stepPhase * 0.5f) * (0.012f + gameWorld_.player.runEffort * 0.030f) +
            breathY - stumbleAmount * 0.055f;
        if (panicActive && viewRuntime_.monsterRunLaunchActive) {
            float launchT = Clamp01(viewRuntime_.monsterRunLaunchMeters / 3.0f);
            float launchWeight = 1.0f - SmoothStep(0.0f, 1.0f, launchT);
            float heavyStep = std::sin(gameWorld_.player.stepPhase * 1.08f + 0.45f);
            float impact = std::abs(std::sin(gameWorld_.player.stepPhase * 0.54f));
            walkY += (heavyStep * 0.185f + impact * 0.105f) * launchWeight;
        }
