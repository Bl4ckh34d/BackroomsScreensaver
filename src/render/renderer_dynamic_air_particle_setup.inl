
        if (!settingsRuntime_.live.airParticles || effectRuntime_.airParticles.empty() || monsterPreview_.active) return;
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        XMFLOAT3 lightOrigin = FlashlightOrigin();
        XMFLOAT3 lightDir = Normalize3(FlashlightForward(), {0.0f, 0.0f, 1.0f});
        XMFLOAT3 cameraForward = Normalize3(DirectionFromYawPitch(world.playerYaw, world.playerPitch), {0.0f, 0.0f, 1.0f});
        float maxDist = std::clamp(settingsRuntime_.live.flashlightShadowDistanceMeters * 0.70f, 5.5f, 12.0f);
        float coneHalf = std::clamp(settingsRuntime_.live.flashlightConeDegrees, 20.0f, 140.0f) * 0.5f * kPi / 180.0f;
        float coneOuter = std::cos(coneHalf);
        float coneInner = std::cos(std::max(3.0f * kPi / 180.0f, coneHalf * 0.50f));
        float levelDensity = AirParticleLevelDensityScale();
        int maxParticles = std::min<int>(static_cast<int>(effectRuntime_.airParticles.size()),
            std::clamp(static_cast<int>(1900.0f * std::clamp(settingsRuntime_.live.airParticleDensity, 0.0f, 4.0f) * levelDensity), 0, 5200));
        int emitted = 0;
        int farParticlePhase = static_cast<int>(timeRuntime_.time * 24.0f) & 1;
        XMFLOAT3 worldUp{0.0f, 1.0f, 0.0f};
        float maxDistSq = maxDist * maxDist;
