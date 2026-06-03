        bool runningToExit = !cameraRuntime_.path.empty() && cameraRuntime_.path.back() == gameWorld_.maze.exit;
        float calmSpeed = IsRoomLike(CameraTile()) ? settingsRuntime_.live.roomSpeed : settingsRuntime_.live.walkSpeed;
        float runTarget = settingsRuntime_.live.runSpeed * (runningToExit ? 1.08f : 0.98f);
        float panicBlend = SmoothStep(0.08f, 0.84f, gameWorld_.monster.chasePanic);
        float speedTarget = Lerp(calmSpeed, runTarget, panicBlend);
        if (softStopActive) speedTarget = 0.0f;
        if (!panicActive && branchLookActive && !softStopActive) {
            speedTarget *= Lerp(1.0f, 0.44f, branchLookWeight);
        }
        if (!panicActive && roomSurveyActive && !softStopActive) {
            speedTarget *= Lerp(1.0f, 0.56f, roomSurveyWeight);
        }
        float dreadSpeedBoost = settingsRuntime_.live.dreadEnabled
            ? viewRuntime_.dreadLevel * (panicActive ? settingsRuntime_.live.dreadRunSpeedBoost * 0.34f : settingsRuntime_.live.dreadWalkSpeedBoost * 0.55f)
            : 0.0f;
        speedTarget *= 1.0f + dreadSpeedBoost;
        speedTarget *= 1.0f - stumbleAmount * 0.16f;
        if (gameWorld_.player.smoothedMoveSpeed <= 0.001f && speedTarget > 0.001f) gameWorld_.player.smoothedMoveSpeed = 0.001f;
        float accel = panicActive ? (viewRuntime_.monsterRunLaunchActive ? 8.0f : 4.2f) : 0.90f;
        float decel = panicActive ? 0.42f : (ventReactionActive ? 1.35f : (softStopActive ? 0.86f : 0.70f));
        gameWorld_.player.smoothedMoveSpeed = MoveTowards(gameWorld_.player.smoothedMoveSpeed, speedTarget,
            (speedTarget > gameWorld_.player.smoothedMoveSpeed ? accel : decel) * dt);
        float speed = gameWorld_.player.smoothedMoveSpeed;
        float moveDistance = std::min(dist, speed * dt);
        float invDist = 1.0f / std::max(0.001f, dist);
        bool movedSafely = MovePlayerThroughCollision(dx * invDist * moveDistance, dz * invDist * moveDistance,
            moveDistance, speed, targetTileForMove, freeRunMove);
        if (movedSafely) {
            if (viewRuntime_.monsterRunLaunchActive) {
                viewRuntime_.monsterRunLaunchMeters = std::min(3.0f, viewRuntime_.monsterRunLaunchMeters + moveDistance);
                if (viewRuntime_.monsterRunLaunchMeters >= 3.0f) viewRuntime_.monsterRunLaunchActive = false;
            }
            if (freeRunMove && moveIndex > cameraRuntime_.pathIndex) {
                cameraRuntime_.pathIndex = moveIndex;
            }
        }
        float runBob = Clamp01((speed - settingsRuntime_.live.walkSpeed) / std::max(0.1f, settingsRuntime_.live.runSpeed * 1.55f - settingsRuntime_.live.walkSpeed));
        gameWorld_.player.runIntensity += (runBob - gameWorld_.player.runIntensity) * std::min(1.0f, dt * (runBob > gameWorld_.player.runIntensity ? (panicActive ? 7.4f : 4.2f) : 1.15f));
        float effortTarget = Clamp01(gameWorld_.player.runIntensity * (panicActive ? 1.18f : 0.82f) + gameWorld_.monster.chasePanic * 0.46f);
        gameWorld_.player.runEffort += (effortTarget - gameWorld_.player.runEffort) * std::min(1.0f, dt * (effortTarget > gameWorld_.player.runEffort ? (panicActive ? 2.4f : 0.55f) : 0.24f));
        gameWorld_.player.breathPhase += dt * (1.15f + gameWorld_.player.runEffort * 4.8f + gameWorld_.player.runIntensity * 1.45f) * kPi;
        if (gameWorld_.player.breathPhase > kPi * 128.0f) {
            gameWorld_.player.breathPhase = std::fmod(gameWorld_.player.breathPhase, kPi * 2.0f);
        }
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
        if (softStopActive) {
            float stopPose = SmoothStep(0.0f, 1.0f, 1.0f - Clamp01(speed / std::max(0.05f, calmSpeed * 0.72f)));
            float idleY = 1.47f + std::sin(timeRuntime_.time * 2.1f) * 0.008f;
            if (ventReactionActive) {
                float ventProgress = 1.0f - viewRuntime_.ventReactionTimer / std::max(0.001f, viewRuntime_.ventReactionDuration);
                float highVent = SmoothStep(0.40f, 0.76f,
                    Clamp01((viewRuntime_.ventReactionTarget.y - 0.36f) / std::max(0.20f, settingsRuntime_.live.wallHeightMeters - 0.72f)));
                float duck = SmoothStep(0.0f, 0.18f, ventReactionElapsed - viewRuntime_.ventReactionLookDelay * 0.45f) *
                    (1.0f - SmoothStep(0.62f, 1.0f, ventProgress)) * highVent;
                float brace = ventBackAwayWeight * (1.0f - highVent) * 0.34f;
                idleY = Lerp(idleY, 1.10f + std::sin(timeRuntime_.time * 8.5f) * 0.006f, duck);
                idleY = Lerp(idleY, 1.40f + std::sin(timeRuntime_.time * 7.2f) * 0.005f, brace);
                stopPose = std::max(stopPose, std::max(duck, ventLookWeight * 0.36f));
            }
            gameWorld_.player.position.y = Lerp(walkY, idleY, stopPose);
        } else {
            gameWorld_.player.position.y = walkY;
        }
