        BloodScarePoint scare{};
        scare.pos = frame.center;
        float sourceY = frame.center.y + height * 0.40f;
        float topY = frame.center.y + height * 0.5f;
        if (topY > liquidBuild.wallH * 0.78f) {
            sourceY = std::max(sourceY, topY - 0.035f);
        }
        scare.source = {frame.center.x, std::clamp(sourceY, 0.18f, liquidBuild.wallH - 0.055f), frame.center.z};
        scare.normal = frame.normal;
        scare.radius = std::clamp(1.25f + std::max(width, height) * 0.72f, 1.75f, 4.35f);
        scare.focusDelaySeconds = 0.30f + LampHash(seed * 43.0f + frame.center.x, frame.center.z) * 0.64f;
        scare.requireFacing = true;
        scare.waterLiquid = waterLiquid;
        if (waterLiquid) {
            scare.dreadScale = 0.30f;
        }
        scareRuntime_.bloodScarePoints.push_back(scare);
        return true;
