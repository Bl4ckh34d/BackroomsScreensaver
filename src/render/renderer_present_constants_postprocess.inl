        cb.post0 = {
            settingsRuntime_.live.exposure,
            settingsRuntime_.live.gamma,
            world.deathActive ? 1.0f : viewRuntime_.dangerLevel,
            deathFadeProgress
        };
        cb.post1 = {
            viewRuntime_.cameraMotionBlur.x,
            viewRuntime_.cameraMotionBlur.y,
            std::clamp(settingsRuntime_.live.bloomAmount, 0.0f, 2.0f),
            std::clamp(settingsRuntime_.live.lensDirtAmount, 0.0f, 2.0f)
        };
        if (sessionRuntime_.mode == RendererRuntimeMode::MainMenu) {
            float menuDoorGlow = SmoothStep(0.08f, 0.82f, exitDoorPresentation_.angle / 1.38f);
            cb.post0.x = Lerp(cb.post0.x, std::max(cb.post0.x, 1.03f), menuDoorGlow);
            cb.post1.z = Lerp(cb.post1.z, std::max(cb.post1.z, 0.34f), menuDoorGlow);
            cb.post1.w = Lerp(cb.post1.w, std::max(cb.post1.w, 0.22f), menuDoorGlow);
        }
        float visionFlashT = scareRuntime_.visionFlashDuration > 0.001f ? Clamp01(scareRuntime_.visionFlashTimer / scareRuntime_.visionFlashDuration) : 0.0f;
        cb.post2 = {
            visionFlashT * visionFlashT,
            0.0f,
            0.0f,
            0.0f
        };
        if (monsterPreview_.active) {
            cb.post0 = {1.0f, 2.2f, 0.0f, 0.0f};
            cb.post1 = {0.0f, 0.0f, 0.0f, 0.0f};
            cb.post2 = {0.0f, 0.0f, 0.0f, 0.0f};
        }
