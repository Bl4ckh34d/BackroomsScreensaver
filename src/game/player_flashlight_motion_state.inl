        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        dt = std::max(0.001f, dt);
        constexpr float kDartFrequencyDivisor = 4.0f;
        float yawDelta = AngleWrap(world.playerYaw - viewRuntime_.previousCameraYaw);
        float pitchDelta = world.playerPitch - viewRuntime_.previousCameraPitch;
        float turnRate = (std::abs(yawDelta) + std::abs(pitchDelta) * 0.65f) / dt;
        float fastTurn = Clamp01((turnRate - 2.1f) / 5.8f);
        float blurScale = monsterPreview_.active ? 0.0f : std::clamp(settingsRuntime_.live.motionBlurAmount, 0.0f, 2.0f);
        XMFLOAT2 blurTarget{
            std::clamp(-yawDelta * 0.36f * blurScale, -0.045f, 0.045f),
            std::clamp(pitchDelta * 0.48f * blurScale, -0.045f, 0.045f)
        };
        float blurFollow = std::min(1.0f, dt * (fastTurn > 0.20f ? 18.0f : 7.0f));
        viewRuntime_.cameraMotionBlur.x += (blurTarget.x - viewRuntime_.cameraMotionBlur.x) * blurFollow;
        viewRuntime_.cameraMotionBlur.y += (blurTarget.y - viewRuntime_.cameraMotionBlur.y) * blurFollow;
        bool calmExplorationTurn = !ChasePanicActive() && viewRuntime_.dangerLevel < 0.16f && DreadPressure() < 0.18f;
        if (fastTurn > 0.0f && !calmExplorationTurn) {
            viewRuntime_.flashlightAgitation = std::max(viewRuntime_.flashlightAgitation, fastTurn);
            viewRuntime_.flashlightDartCooldown = std::min(viewRuntime_.flashlightDartCooldown, RandRange(0.09f, 0.22f) * kDartFrequencyDivisor);
        }

        viewRuntime_.flashlightAgitation = std::max(0.0f, viewRuntime_.flashlightAgitation - dt * 0.42f);
        viewRuntime_.panicFlashlightTimer = std::max(0.0f, viewRuntime_.panicFlashlightTimer - dt);
        viewRuntime_.flashlightDartCooldown = std::max(0.0f, viewRuntime_.flashlightDartCooldown - dt);
        viewRuntime_.flashlightDartTimer = std::max(0.0f, viewRuntime_.flashlightDartTimer - dt);

        float dreadPressure = DreadPressure();
