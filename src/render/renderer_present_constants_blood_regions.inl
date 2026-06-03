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
