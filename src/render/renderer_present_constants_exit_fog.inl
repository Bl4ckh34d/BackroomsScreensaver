// Scene constants for air particles, exit lighting, monster fog, flesh lighting suppression, tessellation, and spark lights.

        cb.air0 = {
            viewRuntime_.airFocusDistance,
            std::clamp(settingsRuntime_.live.airParticleBlur, 0.0f, 3.0f),
            std::clamp(settingsRuntime_.live.airParticleSize, 0.20f, 4.0f),
            settingsRuntime_.live.airParticles ? std::clamp(settingsRuntime_.live.airParticleDensity, 0.0f, 4.0f) : 0.0f
        };
        XMFLOAT3 exitLightPos = exitDoorPresentation_.signLightPosition;
        float exitLightStrength = exitDoorPresentation_.signLightStrength;
        XMFLOAT3 exitLightDir = Normalize3(exitDoorPresentation_.normal, {0.0f, 0.0f, 1.0f});
        float exitDoorOpen = 0.0f;
        XMFLOAT3 doorwayLightPos{0.0f, 0.0f, 0.0f};
        float doorwayLightStrength = 0.0f;
        XMFLOAT3 doorwayPortalPos = Add3(exitDoorPresentation_.center, Scale3(exitLightDir, 0.04f));
        float doorwayPortalHalfWidth = exitLightStrength > 0.001f ? 0.56f : 0.0f;
        if (sessionRuntime_.mode == RendererRuntimeMode::MainMenu && exitDoorPresentation_.angle > 0.001f) {
            float rawDoorOpen = exitDoorPresentation_.angle / 1.38f;
            exitDoorOpen = SmoothStep(0.08f, 0.92f, rawDoorOpen);
            float doorwayLightOpen = SmoothStep(0.12f, 0.90f, rawDoorOpen);
            doorwayLightOpen *= doorwayLightOpen;
            XMFLOAT3 stairSource = Scale3(exitDoorPresentation_.normal, -2.65f);
            doorwayLightPos = Add3(exitDoorPresentation_.center, stairSource);
            doorwayLightPos.y = exitDoorPresentation_.center.y + 4.35f;
            doorwayLightStrength = doorwayLightOpen * 5.8f;
            doorwayPortalPos = Add3(exitDoorPresentation_.center, Scale3(exitDoorPresentation_.normal, 0.04f));
            doorwayPortalHalfWidth = 0.56f;
        }
        cb.exitLight0 = {
            exitLightPos.x,
            exitLightPos.y,
            exitLightPos.z,
            exitLightStrength
        };
        cb.exitLight1 = {
            exitLightDir.x,
            exitLightDir.y,
            exitLightDir.z,
            exitDoorOpen
        };
        cb.exitLight2 = {
            doorwayLightPos.x,
            doorwayLightPos.y,
            doorwayLightPos.z,
            doorwayLightStrength
        };
        cb.exitLight3 = {
            doorwayPortalPos.x,
            doorwayPortalPos.y,
            doorwayPortalPos.z,
            doorwayPortalHalfWidth
        };
        float monsterFogRadius = std::max(tileAverage * 2.60f, 5.80f);
        float monsterFogStrength = (monsterPreview_.active || gEffectDebugViewer || gBloodDebugEveryWall || settingsRuntime_.live.bloodStudyView)
            ? 0.0f
            : 0.72f;
        cb.monsterFog0 = {
            world.monsterPosition.x,
            world.monsterPosition.z,
            monsterFogRadius,
            monsterFogStrength
        };
        bool fleshLightingSuppressed =
            (settingsRuntime_.live.fleshAlwaysOn && settingsRuntime_.live.fleshFlickerIntensity > 0.001f) ||
            (scareRuntime_.fleshFlickerTimer > 0.0f && scareRuntime_.fleshFlickerDuration > 0.001f);
        if (fleshLightingSuppressed) {
            // Shader uses transition0.z to keep the entire flesh event flashlight-only, including strobe gaps.
            cb.transition0.z = 1.0f;
            cb.lighting0.z = 0.0f;
            cb.lighting1.x = 0.0f;
            cb.lighting1.y = 0.0f;
            cb.lighting1.z = 0.0f;
        }
        bool useFleshTessellation = cb.horror0.x > 0.01f &&
            d3dRuntime_.featureLevel >= D3D_FEATURE_LEVEL_11_0 &&
            shaders_.hullShader && shaders_.domainShader &&
            cb.horror0.w > 0.001f;
        std::array<XMFLOAT4, 2> sparkLights = fleshLightingSuppressed
            ? std::array<XMFLOAT4, 2>{XMFLOAT4{0.0f, 0.0f, 0.0f, 0.0f}, XMFLOAT4{0.0f, 0.0f, 0.0f, 0.0f}}
            : ActiveSparkLights();
        cb.sparkLight0 = sparkLights[0];
        cb.sparkLight1 = sparkLights[1];

