        float turnSpeed = panicActive ? Lerp(4.2f, 6.2f, chaseLookBackWeight) : (bloodFocusActive ? 4.1f : 2.9f);
        if (ventReactionActive) turnSpeed = Lerp(2.6f, 7.8f, ventLookWeight);
        else if (branchLookActive) turnSpeed = cameraRuntime_.branchLookPaused ? 5.25f : 4.65f;
        else if (roomSurveyActive) turnSpeed = softStopActive ? 3.65f : 2.85f;
        else if (!panicActive && pathTurnWeight > 0.001f) turnSpeed = Lerp(turnSpeed, 4.15f, Clamp01(pathTurnWeight / 0.68f));
        if (!panicActive && !softStopActive && viewRuntime_.exitLookBlend > 0.001f) {
            turnSpeed = std::max(turnSpeed, Lerp(3.45f, 4.85f, Clamp01(viewRuntime_.exitLookBlend / 0.82f)));
        }
        gameWorld_.player.yaw += AngleWrap(desiredYaw - gameWorld_.player.yaw) * std::min(1.0f, dt * turnSpeed);
        float pitchTarget = panicActive ? -0.030f : -0.055f;
        if (ventReactionActive) {
            float ventTargetPitch = std::clamp(PitchToPoint(viewRuntime_.ventReactionTarget), -0.58f, 0.24f);
            float scanWeight = SmoothStep(0.10f, 1.0f, ventLookWeight);
            float scanPitch = (std::sin(timeRuntime_.time * 8.6f + viewRuntime_.ventReactionScanSeed * 1.3f) * 0.024f +
                std::sin(timeRuntime_.time * 15.2f + viewRuntime_.ventReactionScanSeed) * 0.010f) * scanWeight;
            pitchTarget = Lerp(gameWorld_.player.pitch, ventTargetPitch, ventLookWeight) + scanPitch;
        } else if (bloodFocusActive) {
            pitchTarget = std::clamp(PitchToPoint(scareRuntime_.bloodFocusTarget), -0.34f, 0.22f);
        } else if (threat && chaseLookBackWeight > 0.0f) {
            pitchTarget = Lerp(pitchTarget, viewRuntime_.chaseLookBackPitch, chaseLookBackWeight);
        } else if (branchLookActive) {
            pitchTarget = Lerp(pitchTarget, cameraRuntime_.branchLookPitch, branchLookWeight * (cameraRuntime_.branchLookPaused ? 0.88f : 0.72f));
        } else if (roomSurveyActive) {
            pitchTarget = Lerp(pitchTarget, RoomSurveyPitch(), roomSurveyWeight * 0.78f);
        }
        if (!panicActive && !softStopActive && viewRuntime_.exitLookBlend > 0.001f) {
            float exitPitch = std::clamp(PitchToPoint(viewRuntime_.exitLookFocus), -0.24f, 0.26f);
            pitchTarget = Lerp(pitchTarget, exitPitch, std::min(0.92f, viewRuntime_.exitLookBlend * 0.92f));
        }
        pitchTarget -= stumbleAmount * 0.035f * (1.0f - chaseLookBackWeight * 0.85f);
        gameWorld_.player.pitch += (pitchTarget - gameWorld_.player.pitch) * std::min(1.0f, dt * (panicActive ? 3.5f : (ventReactionActive ? Lerp(2.0f, 7.2f, ventLookWeight) : 2.2f)));
