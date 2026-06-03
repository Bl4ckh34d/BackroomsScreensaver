        cb.shadow0 = {
            lightPosFloat.x,
            lightPosFloat.y,
            lightPosFloat.z,
            settingsRuntime_.live.flashlightShadows ? settingsRuntime_.live.flashlightShadowStrength : 0.0f
        };
        cb.shadow1 = {
            lightDirFloat.x,
            lightDirFloat.y,
            lightDirFloat.z,
            settingsRuntime_.live.flashlightShadowBias
        };
        cb.shadow2 = {
            1.0f / static_cast<float>(std::max<UINT>(1, shadowResources_.shadowMapSize)),
            shadowDistance,
            coneOuter,
            coneInner
        };
        cb.fixtureShadow0 = {
            fixtureShadowPos.x,
            fixtureShadowPos.y,
            fixtureShadowPos.z,
            fixtureShadowActive ? std::clamp(settingsRuntime_.live.flashlightShadowStrength * 0.72f, 0.0f, 0.62f) : 0.0f
        };
        cb.fixtureShadow1 = {
            std::max(settingsRuntime_.live.flashlightShadowBias * 1.35f, 0.00062f),
            fixtureShadowRange,
            1.0f / static_cast<float>(std::max<UINT>(1, shadowResources_.fixtureShadowMapSize)),
            0.0f
        };
        fixtureShadowRuntime_.active = fixtureShadowActive;
        fixtureShadowRuntime_.position = fixtureShadowPos;
        fixtureShadowRuntime_.range = fixtureShadowRange;
        cb.maze0 = {
            -static_cast<float>(lightingMaze.w) * lightingMaze.tileW * 0.5f,
            -static_cast<float>(lightingMaze.h) * lightingMaze.tileD * 0.5f,
            lightingMaze.tileW,
            lightingMaze.tileD
        };
        cb.maze1 = {
            static_cast<float>(lightingMaze.w),
            static_cast<float>(lightingMaze.h),
            settingsRuntime_.live.wallHeightMeters,
            tileAverage
        };
        float dirtProgression = 0.48f;
        if (IsPlayableSimulationMode(sessionRuntime_.mode) && gameWorld_.PlayableRunActive()) {
            dirtProgression = gameWorld_.MapDirtProgression();
        } else if (sessionRuntime_.mode == RendererRuntimeMode::MainMenu) {
            dirtProgression = menuRuntime_.darkLayerOneRun ? 0.72f : 0.42f;
        } else if (gEffectDebugViewer && DebugSliceEffectIsWater(gDebugSliceEffect)) {
            dirtProgression = 0.90f;
        } else {
            float damagePressure = (settingsRuntime_.live.waterDamageEnabled ? 0.24f : 0.0f) +
                std::clamp(settingsRuntime_.live.waterDamageDensity, 0.0f, 4.0f) * 0.16f +
                std::clamp(settingsRuntime_.live.bloodSplatterDensity, 0.0f, 4.0f) * 0.12f +
                std::clamp(settingsRuntime_.live.jumpscareFrequency, 0.0f, 1.0f) * 0.12f +
                std::clamp(settingsRuntime_.live.brokenZoneRatio, 0.0f, 1.0f) * 0.18f;
            dirtProgression = Clamp01(0.34f + damagePressure);
        }
        cb.texture0 = {
            settingsRuntime_.live.wallTextureMeters,
            settingsRuntime_.live.floorTextureMeters,
            settingsRuntime_.live.ceilingTextureMeters,
            dirtProgression
        };
        cb.transition0 = {
            transitionFade,
            world.exitTransitionActive ? exitDoorPresentation_.angle : 0.0f,
            0.0f,
            0.0f
        };
        if (gEffectDebugViewer && DebugSliceEffectIsWater(gDebugSliceEffect)) {
            cb.transition0.w = 1.0f + DebugSliceLoopPhase();
        } else if (!gEffectDebugViewer) {
            cb.transition0.w = 0.0f;
        }
