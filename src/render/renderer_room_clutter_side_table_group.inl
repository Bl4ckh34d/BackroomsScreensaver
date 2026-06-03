        if (kind < 0.62f) {
            float sideW = 1.12f + Rand01(groupIndex, 379, scatterSeed) * 0.44f;
            float sideD = 0.66f + Rand01(groupIndex, 383, scatterSeed) * 0.30f;
            float chairRadius = std::max(sideW, sideD) * 0.5f + 0.76f;
            if (!longFootprintClear(px, pz, chairRadius * 2.0f + 0.80f, chairRadius * 1.55f, yaw, 0.070f)) return false;
            float sideSeed = Rand01(groupIndex, 389, scatterSeed);
            if (!AddSideTableProp(build, px, pz, sideW, sideD, yaw, sideSeed)) return false;
            if (Rand01(groupIndex, 391, scatterSeed) < 0.58f) {
                AddDeskLampOnSurfaceProp(build, px, pz, 0.720f + sideSeed * 0.10f, yaw, sideW, sideD, Rand01(groupIndex, 393, scatterSeed));
            }
            AddTippedChairProp(build, px + std::sin(yaw + 0.8f) * chairRadius, pz + std::cos(yaw + 0.8f) * chairRadius,
                yaw + kPi * 0.72f, Rand01(groupIndex, 397, scatterSeed) < 0.5f, Rand01(groupIndex, 401, scatterSeed));
            float binA = yaw - 0.95f + (Rand01(groupIndex, 403, scatterSeed) - 0.5f) * 0.58f;
            AddTrashBinProp(build, px + std::sin(binA) * chairRadius * 0.72f, pz + std::cos(binA) * chairRadius * 0.72f,
                binA + kPi * 0.5f, Rand01(groupIndex, 405, scatterSeed) < 0.52f,
                Rand01(groupIndex, 407, scatterSeed));
            return true;
        }
