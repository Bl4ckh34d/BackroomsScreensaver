        float flashlightIntensity = settingsRuntime_.live.flashlightIntensity *
            (sessionRuntime_.mode == RendererRuntimeMode::MainMenu ? 1.0f : DreadFlashlightMultiplier());
        if (sessionRuntime_.mode == RendererRuntimeMode::PlayableGame && !world.playerFlashlightEnabled) flashlightIntensity = 0.0f;
        if (monsterPreview_.active) flashlightIntensity = 1.45f;
        cb.cameraPosTime = {eyePos.x, eyePos.y, eyePos.z, timeRuntime_.time};
        cb.cameraDirAspect = {lightDirFloat.x, lightDirFloat.y, lightDirFloat.z, aspect};
        cb.lighting0 = {
            flashlightIntensity,
            settingsRuntime_.live.flashlightAttenuation,
            settingsRuntime_.live.ambientLight,
            std::max(2.0f, settingsRuntime_.live.lampSpacing)
        };
        if (sessionRuntime_.mode == RendererRuntimeMode::MainMenu) {
            float menuDoorAmbient = SmoothStep(0.04f, 0.78f, exitDoorPresentation_.angle / 1.38f);
            cb.lighting0.z = Lerp(std::max(settingsRuntime_.live.ambientLight, 0.045f), 0.160f, menuDoorAmbient);
        }
        if (monsterPreview_.active) {
            cb.lighting0.y = 0.030f;
            cb.lighting0.z = 0.22f;
        }
        if (gEffectDebugViewer) {
            cb.lighting0.x = std::clamp(cb.lighting0.x, 0.55f, 0.92f);
            cb.lighting0.y = std::max(cb.lighting0.y, 0.085f);
            cb.lighting0.z = kEffectDebugAmbientLight;
        } else if (gBloodDebugEveryWall || settingsRuntime_.live.bloodStudyView) {
            cb.lighting0.x = std::max(cb.lighting0.x, 1.85f);
            cb.lighting0.y = std::min(cb.lighting0.y, 0.040f);
            cb.lighting0.z = std::max(cb.lighting0.z, 0.42f);
        }
        cb.lighting1 = {
            settingsRuntime_.live.lampIntensity,
            settingsRuntime_.live.lampOnRatio,
            settingsRuntime_.live.lampFlickerRatio,
            settingsRuntime_.live.brokenZoneRatio
        };
        if (gEffectDebugViewer) {
            cb.lighting1.x = (gDebugSliceEffect == DebugSliceEffect::CeilingLamps) ? kEffectDebugLampIntensity : 0.0f;
            cb.lighting1.y = (gDebugSliceEffect == DebugSliceEffect::CeilingLamps) ? 1.0f : 0.0f;
            cb.lighting1.z = 0.0f;
            cb.lighting1.w = (gDebugSliceEffect == DebugSliceEffect::BrokenLamps) ? 1.0f : 0.0f;
        } else if (gBloodDebugEveryWall || settingsRuntime_.live.bloodStudyView) {
            cb.lighting1.x = std::max(cb.lighting1.x, 2.35f);
            cb.lighting1.y = 1.0f;
            cb.lighting1.z = 0.0f;
            cb.lighting1.w = 0.0f;
        }
        cb.fog0 = {
            settingsRuntime_.live.fogStartMeters,
            settingsRuntime_.live.fogEndMeters,
            settingsRuntime_.live.fogDarkness,
            0.0f
        };
        if (monsterPreview_.active) {
            cb.fog0 = {1000.0f, 1001.0f, 0.0f, 0.0f};
        }
        if (gEffectDebugViewer || gBloodDebugEveryWall || settingsRuntime_.live.bloodStudyView) {
            cb.fog0 = {1000.0f, 1001.0f, 0.0f, 0.0f};
        }
        cb.ao0 = {
            settingsRuntime_.live.cornerAOIntensity,
            settingsRuntime_.live.cornerAORadius,
            settingsRuntime_.live.floorCeilingAOIntensity,
            tileAverage
        };
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
