
        BloodScarePoint scare{};
        scare.pos = {floorCenter.x, 0.10f, floorCenter.z};
        scare.source = {wallFrame.center.x, liquidBuild.wallH - 0.035f, wallFrame.center.z};
        scare.normal = wallFrame.normal;
        scare.radius = std::clamp(1.95f + std::max(poolW, poolD) * 0.96f, 2.65f, 5.65f);
        scare.focusDelaySeconds = 0.34f + Rand01(leakIndex, 733, scatterSeed) * 0.66f;
        scare.requireFacing = true;
        scare.waterLiquid = waterLiquid;
        if (waterLiquid) {
            scare.dreadScale = 0.30f;
        }
        scareRuntime_.bloodScarePoints.push_back(scare);
        return true;
