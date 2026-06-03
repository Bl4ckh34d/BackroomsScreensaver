    bool MatureWaterRevealContains(float x, float z, float radius = 0.0f) const {
        for (const BloodRevealRegion& region : scareRuntime_.bloodRevealRegions) {
            if (!region.waterLiquid || region.radius <= 0.01f || region.activationTime <= -999000.0f) continue;
            float age = timeRuntime_.time - region.activationTime;
            if (age < 5.8f) continue;
            float grow = SmoothStep(5.8f, 24.0f, age);
            float activeRadius = Lerp(std::min(region.radius, 0.42f), region.radius * 0.86f, grow);
            float dx = x - region.center.x;
            float dz = z - region.center.z;
            float effectiveRadius = activeRadius + radius;
            if (dx * dx + dz * dz <= effectiveRadius * effectiveRadius) {
                return true;
            }
        }
        return false;
    }
