// Scene constants for flesh flicker, blood world, blood reveal regions, and blood shader tuning.

        float fleshAmount = 0.0f;
        if (scareRuntime_.fleshFlickerTimer > 0.0f && scareRuntime_.fleshFlickerDuration > 0.001f) {
            float elapsed = scareRuntime_.fleshFlickerDuration - scareRuntime_.fleshFlickerTimer;
            float phase = Clamp01(elapsed / scareRuntime_.fleshFlickerDuration);
            float envelope = SmoothStep(0.0f, 0.045f, elapsed) * (1.0f - SmoothStep(scareRuntime_.fleshFlickerDuration - 0.10f, scareRuntime_.fleshFlickerDuration, elapsed));
            float strobe = (std::sin(phase * kPi * 4.0f) > 0.0f) ? 1.0f : 0.0f;
            fleshAmount = envelope * strobe * settingsRuntime_.live.fleshFlickerIntensity;
        }
        if (settingsRuntime_.live.fleshAlwaysOn) fleshAmount = std::max(fleshAmount, settingsRuntime_.live.fleshFlickerIntensity);
        cb.horror0 = {
            Clamp01(fleshAmount),
            std::clamp(settingsRuntime_.live.bloodWetness, 0.0f, 3.0f),
            std::clamp(settingsRuntime_.live.fleshWetness, 0.0f, 4.0f),
            std::clamp(settingsRuntime_.live.fleshParallaxScale, 0.0f, 0.50f)
        };
        float bloodStreamCount = static_cast<float>(std::clamp(settingsRuntime_.live.bloodStreamCount, 4, 32));
        float bloodStreamThickness = std::clamp(settingsRuntime_.live.bloodStreamThickness, 0.10f, 2.0f);
        float bloodShaderQuality = std::clamp(settingsRuntime_.live.bloodShaderQuality, 0.25f, 1.0f);
        float bloodWorldAmount = 0.0f;
        if (scareRuntime_.bloodWorldFlickerTimer > 0.0f && scareRuntime_.bloodWorldFlickerDuration > 0.001f) {
            float elapsed = scareRuntime_.bloodWorldFlickerDuration - scareRuntime_.bloodWorldFlickerTimer;
            float envelope = SmoothStep(0.0f, 0.055f, elapsed) *
                (1.0f - SmoothStep(scareRuntime_.bloodWorldFlickerDuration - 0.18f, scareRuntime_.bloodWorldFlickerDuration, elapsed));
            float strobe = ((std::sin(elapsed * 41.0f) + std::sin(elapsed * 93.0f) * 0.48f + std::sin(elapsed * 151.0f) * 0.22f) > -0.06f) ? 1.0f : 0.0f;
            bloodWorldAmount = envelope * strobe * settingsRuntime_.live.bloodWorldFlickerIntensity;
        }
        if (settingsRuntime_.live.bloodWorldAlwaysOn && settingsRuntime_.live.bloodWorldCoverage > 0.001f) {
            bloodWorldAmount = std::max(bloodWorldAmount, settingsRuntime_.live.bloodWorldFlickerIntensity);
            if (scareRuntime_.bloodWorldActivationTime < -900.0f) scareRuntime_.bloodWorldActivationTime = timeRuntime_.time - 46.0f;
        }
        if (sessionRuntime_.mode == RendererRuntimeMode::MainMenu && settingsRuntime_.live.bloodWorldCoverage > 0.001f) {
            bloodWorldAmount = std::max(bloodWorldAmount, menuRuntime_.bloodAmount);
        }
        cb.blood0 = {0.0f, 0.0f, 0.0f, 0.0f};
        cb.blood1 = {0.0f, 0.0f, 0.0f, 0.0f};
        cb.blood2 = {0.0f, 0.0f, 0.0f, 0.0f};
        cb.blood3 = {0.0f, 0.0f, 0.0f, 0.0f};
        cb.blood4 = {0.0f, 0.0f, 0.0f, 0.0f};
        cb.blood5 = {0.0f, 0.0f, 0.0f, 0.0f};
        cb.blood6 = {0.0f, 0.0f, 0.0f, 0.0f};
        cb.blood7 = {0.0f, 0.0f, 0.0f, 0.0f};
        cb.blood8 = {0.0f, 0.0f, 0.0f, 0.0f};
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
        } else if (!scareRuntime_.bloodRevealRegions.empty()) {
            std::array<const BloodRevealRegion*, 8> nearestRegions{};
            std::array<float, 8> nearestDistSq{};
            nearestDistSq.fill(std::numeric_limits<float>::infinity());
            size_t nearestCount = 0;
            for (const BloodRevealRegion& region : scareRuntime_.bloodRevealRegions) {
                if (region.radius <= 0.001f || region.activationTime <= -999000.0f) continue;
                float dx = region.center.x - world.playerPosition.x;
                float dz = region.center.z - world.playerPosition.z;
                float distSq = dx * dx + dz * dz;
                if (nearestCount < nearestRegions.size()) {
                    nearestRegions[nearestCount] = &region;
                    nearestDistSq[nearestCount] = distSq;
                    ++nearestCount;
                } else {
                    size_t slot = nearestRegions.size() - 1;
                    for (size_t i = 0; i < nearestRegions.size(); ++i) {
                        if (nearestDistSq[i] > nearestDistSq[slot]) slot = i;
                    }
                    if (distSq >= nearestDistSq[slot]) continue;
                    nearestRegions[slot] = &region;
                    nearestDistSq[slot] = distSq;
                }
            }
            for (size_t i = 1; i < nearestCount; ++i) {
                const BloodRevealRegion* region = nearestRegions[i];
                float distSq = nearestDistSq[i];
                size_t j = i;
                while (j > 0 && distSq < nearestDistSq[j - 1]) {
                    nearestRegions[j] = nearestRegions[j - 1];
                    nearestDistSq[j] = nearestDistSq[j - 1];
                    --j;
                }
                nearestRegions[j] = region;
                nearestDistSq[j] = distSq;
            }
            if (nearestCount > 0 && nearestRegions[0]) {
                const BloodRevealRegion& primary = *nearestRegions[0];
                cb.blood0 = {primary.center.x, primary.center.z, primary.activationTime, 1.0f};
                cb.blood1 = {bloodStreamCount, std::max(1.0f, primary.radius), bloodStreamThickness, bloodShaderQuality};
                auto assignRegion = [&](size_t slot, const BloodRevealRegion& region) {
                    XMFLOAT4 value{
                        region.center.x,
                        region.center.z,
                        region.activationTime,
                        std::max(1.0f, region.radius)
                    };
                    switch (slot) {
                    case 2: cb.blood2 = value; break;
                    case 3: cb.blood3 = value; break;
                    case 4: cb.blood4 = value; break;
                    case 5: cb.blood5 = value; break;
                    case 6: cb.blood6 = value; break;
                    case 7: cb.blood7 = value; break;
                    case 8: cb.blood8 = value; break;
                    default: break;
                    }
                };
                size_t slot = 2;
                for (size_t i = 1; i < nearestCount && slot <= 8; ++i, ++slot) {
                    if (nearestRegions[i]) assignRegion(slot, *nearestRegions[i]);
                }
            }
        }
