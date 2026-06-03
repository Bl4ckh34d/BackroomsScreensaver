        if (!wallOnly && !gEffectDebugViewer) {
            BloodScarePoint scare{};
            scare.pos = {floorCenter.x, 0.10f, floorCenter.z};
            scare.source = {wallFrame.center.x, wallH - 0.035f, wallFrame.center.z};
            scare.normal = wallFrame.normal;
            scare.radius = std::clamp(1.65f + std::max(leakW, poolD) * 0.70f, 2.10f, 4.80f);
            scare.focusDelaySeconds = 0.22f + Rand01(leakIndex, 2433, scatterSeed) * 0.44f;
            scare.requireFacing = true;
            scare.dreadScale = 0.42f;
            scare.revealBlood = false;
            scareRuntime_.bloodScarePoints.push_back(scare);
        }
