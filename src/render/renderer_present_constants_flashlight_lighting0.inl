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
