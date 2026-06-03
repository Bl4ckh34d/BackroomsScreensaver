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
