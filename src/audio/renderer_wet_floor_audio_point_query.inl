    bool IsWetFootstepPoint(float x, float z, float radius = 0.08f) const {
        if (!MatureWaterRevealContains(x, z, radius)) return false;
        for (const WetFloorFootprint& fp : effectRuntime_.wetFloorFootprints) {
            if (fp.wetDelaySeconds > 0.0f) {
                bool delayedEnough = false;
                for (const BloodRevealRegion& region : scareRuntime_.bloodRevealRegions) {
                    if (!region.waterLiquid || region.activationTime <= -999000.0f) continue;
                    if (timeRuntime_.time - region.activationTime >= fp.wetDelaySeconds) {
                        delayedEnough = true;
                        break;
                    }
                }
                if (!delayedEnough) continue;
            }
            float dx = x - fp.center.x;
            float dz = z - fp.center.y;
            float localX = dx * fp.right.x + dz * fp.right.y;
            float localZ = dx * fp.forward.x + dz * fp.forward.y;
            if (std::abs(localX) <= fp.halfW + radius && std::abs(localZ) <= fp.halfD + radius) {
                return true;
            }
        }
        return false;
    }
