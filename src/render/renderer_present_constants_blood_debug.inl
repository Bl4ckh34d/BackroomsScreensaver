        if (gBloodDebugEveryWall) {
            float debugClock = DebugSliceClockSeconds();
            float cycleAge = std::fmod(std::max(0.0f, debugClock), std::max(0.1f, settingsRuntime_.live.effectBloodLoopSeconds));
            float debugAge = std::min(cycleAge, settingsRuntime_.live.effectBloodFullSpreadAge);
            cb.blood0 = {0.0f, 0.0f, timeRuntime_.time - debugAge, 1.0f};
            cb.blood1 = {bloodStreamCount, kEffectBloodRevealRadius, bloodStreamThickness, bloodShaderQuality};
        } else if (settingsRuntime_.live.bloodStudyView) {
            cb.blood0 = {0.0f, 0.0f, -40.0f, 1.0f};
            cb.blood1 = {bloodStreamCount, kEffectBloodRevealRadius, bloodStreamThickness, bloodShaderQuality};
        } else if (bloodWorldAmount > 0.001f && settingsRuntime_.live.bloodWorldCoverage > 0.001f) {
            XMFLOAT3 bloodCenter = {world.playerPosition.x, 0.0f, world.playerPosition.z};
            float revealRadius = kEffectBloodRevealRadius;
            if (sessionRuntime_.mode == RendererRuntimeMode::MainMenu) {
                XMFLOAT3 c = lightingMaze.WorldCenter(lightingMaze.start, 0.0f);
                bloodCenter = {c.x + lightingMaze.tileW * 0.14f, 0.0f, c.z + lightingMaze.tileD * 0.50f};
                revealRadius = std::max(2.35f, std::max(lightingMaze.tileW, lightingMaze.tileD) * 1.35f);
            }
            cb.blood0 = {bloodCenter.x, bloodCenter.z, scareRuntime_.bloodWorldActivationTime, Clamp01(bloodWorldAmount)};
            cb.blood1 = {bloodStreamCount, revealRadius, bloodStreamThickness, bloodShaderQuality};
