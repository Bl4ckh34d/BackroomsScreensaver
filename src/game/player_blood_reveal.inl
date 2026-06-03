// Blood reveal and proximity-pressure scare helpers. 
// Included inside Renderer's private section from player_monster_pressure.inl.

    void IncludeBloodReveal(const BloodScarePoint& point) {
        BloodRevealRegion region{};
        region.center = point.pos;
        float tileMin = std::max(0.1f, RenderMazeView().TileMinimum());
        float tileAvg = std::max(0.1f, RenderMazeView().TileAverage());
        float maxRadius = point.waterLiquid ? tileAvg * 2.65f : tileAvg * 1.35f;
        region.radius = std::clamp(point.radius, tileMin * 0.72f, maxRadius);
        region.activationTime = point.activationTime;
        region.waterLiquid = point.waterLiquid;

        for (BloodRevealRegion& existing : scareRuntime_.bloodRevealRegions) {
            float dx = existing.center.x - region.center.x;
            float dz = existing.center.z - region.center.z;
            float distSq = dx * dx + dz * dz;
            float mergeLimit = tileMin * 0.24f;
            if (point.waterLiquid && existing.waterLiquid) {
                mergeLimit = std::max(tileAvg * 1.85f, std::min(existing.radius, region.radius) * 0.92f);
            } else if (point.waterLiquid != existing.waterLiquid) {
                mergeLimit = tileMin * 0.18f;
            }
            if (distSq <= mergeLimit * mergeLimit) {
                float dist = std::sqrt(distSq);
                if (point.waterLiquid && existing.waterLiquid && dist > 0.001f) {
                    float existingWeight = std::max(existing.radius, 0.1f);
                    float regionWeight = std::max(region.radius, 0.1f);
                    float totalWeight = existingWeight + regionWeight;
                    existing.center.x = (existing.center.x * existingWeight + region.center.x * regionWeight) / totalWeight;
                    existing.center.z = (existing.center.z * existingWeight + region.center.z * regionWeight) / totalWeight;
                    existing.radius = std::clamp(std::max(existing.radius, region.radius) + dist * 0.55f,
                        tileMin * 0.72f, maxRadius);
                } else {
                    existing.radius = std::max(existing.radius, region.radius);
                }
                existing.activationTime = std::min(existing.activationTime, region.activationTime);
                return;
            }
        }

        scareRuntime_.bloodRevealRegions.push_back(region);
        constexpr size_t kMaxBloodRevealRegions = 96;
        if (scareRuntime_.bloodRevealRegions.size() > kMaxBloodRevealRegions) {
            scareRuntime_.bloodRevealRegions.erase(scareRuntime_.bloodRevealRegions.begin());
        }
    }
