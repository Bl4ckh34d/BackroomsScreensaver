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
            float rawDoorOpen = std::clamp(exitDoorPresentation_.angle / 1.38f, 0.0f, 1.0f);
            exitDoorOpen = SmoothStep(0.08f, 0.92f, rawDoorOpen);
            float doorwayLightOpen = SmoothStep(0.12f, 0.90f, rawDoorOpen);
            doorwayLightOpen *= doorwayLightOpen;
            XMFLOAT3 stairSource = Scale3(exitDoorPresentation_.normal, -2.65f);
            doorwayLightPos = Add3(exitDoorPresentation_.center, stairSource);
            doorwayLightPos.y = exitDoorPresentation_.center.y + 4.35f;
            doorwayLightStrength = doorwayLightOpen * 5.8f;
            doorwayPortalPos = Add3(exitDoorPresentation_.center, Scale3(exitDoorPresentation_.normal, 0.04f));
            doorwayPortalHalfWidth = 0.56f;
        } else if (exitDoorPresentation_.angle > 0.001f) {
            float rawDoorOpen = std::clamp(exitDoorPresentation_.angle / 1.38f, 0.0f, 1.0f);
            exitDoorOpen = SmoothStep(0.08f, 0.92f, rawDoorOpen);
            doorwayPortalPos = Add3(exitDoorPresentation_.center, Scale3(exitDoorPresentation_.normal, 0.04f));
            doorwayPortalHalfWidth = 0.56f;
            doorwayLightPos = Add3(exitDoorPresentation_.center, Scale3(exitDoorPresentation_.normal, 2.0f));
            float doorwayBase = std::max(settingsRuntime_.live.lampIntensity, exitLightStrength * 0.26f);
            doorwayLightStrength = std::clamp(doorwayBase * 0.78f, 0.0f, 1.35f);
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
