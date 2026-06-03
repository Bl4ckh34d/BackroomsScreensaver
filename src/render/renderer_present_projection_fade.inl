        float aspect = static_cast<float>(std::max<LONG>(1, hostRuntime_.width)) / static_cast<float>(std::max<LONG>(1, hostRuntime_.height));
        float fovDegrees = monsterPreview_.active ? 48.0f : 70.0f - viewRuntime_.dangerLevel * 3.5f;
        if (sessionRuntime_.mode == RendererRuntimeMode::MainMenu) {
            fovDegrees = 84.0f;
        }
        if (gEffectDebugViewer || gBloodDebugEveryWall || settingsRuntime_.live.bloodStudyView) {
            fovDegrees = 86.0f;
        }
        if (world.deathActive) {
            fovDegrees = Lerp(70.0f, 58.0f, SmoothStep(0.05f, 0.66f, deathProgress));
        }
        float exitStepStart = ExitAlignSeconds() + std::max(0.05f, settingsRuntime_.live.exitDoorOpenSeconds * 0.68f);
        float exitStepEnd = exitStepStart + settingsRuntime_.live.exitStepSeconds;
        float exitFade = world.exitTransitionActive ? SmoothStep(exitStepEnd - settingsRuntime_.live.exitFadeSeconds * 0.64f, exitStepEnd + settingsRuntime_.live.exitFadeSeconds * 0.36f, world.exitTransitionTimer) : 0.0f;
        float fadeIn = (viewRuntime_.fadeInTimer > 0.0f && settingsRuntime_.live.fadeInSeconds > 0.001f)
            ? 1.0f - SmoothStep(0.0f, settingsRuntime_.live.fadeInSeconds, settingsRuntime_.live.fadeInSeconds - viewRuntime_.fadeInTimer)
            : 0.0f;
        float transitionFade = std::max(std::max(exitFade, fadeIn), menuRuntime_.startTransitionFade);
        float viewFarMeters = monsterPreview_.active ? 1000.0f : std::max(72.0f, settingsRuntime_.live.fogEndMeters + tileAverage * 12.0f);
        XMMATRIX proj = XMMatrixPerspectiveFovLH(fovDegrees * kPi / 180.0f, aspect, 0.045f, viewFarMeters);
