        if (kind < 0.82f && !renderAssets_.trashBinPropMesh.vertices.empty()) {
            float clusterRadius = 0.70f + Rand01(groupIndex, 409, scatterSeed) * 0.36f;
            if (!longFootprintClear(px, pz, clusterRadius * 2.0f + 1.10f, clusterRadius * 2.0f + 1.00f, yaw, 0.070f)) return false;
            float furnitureYaw = yaw + (Rand01(groupIndex, 411, scatterSeed) - 0.5f) * 0.85f;
            float furnitureA = yaw + kPi * (0.40f + Rand01(groupIndex, 413, scatterSeed) * 0.40f);
            if (Rand01(groupIndex, 415, scatterSeed) < 0.45f) {
                float tableW = 0.82f + Rand01(groupIndex, 417, scatterSeed) * 0.26f;
                float tableD = 0.52f + Rand01(groupIndex, 419, scatterSeed) * 0.20f;
                float smallTableSeed = Rand01(groupIndex, 421, scatterSeed);
                float tableX = px + std::sin(furnitureA) * 0.38f;
                float tableZ = pz + std::cos(furnitureA) * 0.38f;
                if (AddSideTableProp(build, tableX, tableZ, tableW, tableD, furnitureYaw, smallTableSeed) &&
                    Rand01(groupIndex, 422, scatterSeed) < 0.46f) {
                    AddDeskLampOnSurfaceProp(build, tableX, tableZ, 0.720f + smallTableSeed * 0.10f,
                        furnitureYaw, tableW, tableD, Rand01(groupIndex, 424, scatterSeed));
                }
            } else if (Rand01(groupIndex, 423, scatterSeed) < 0.55f) {
                AddChairProp(build, {px + std::sin(furnitureA) * 0.48f, 0.0f, pz + std::cos(furnitureA) * 0.48f},
                    furnitureYaw + kPi, Rand01(groupIndex, 425, scatterSeed) < 0.5f);
            }
            int bins = 2 + static_cast<int>(Rand01(groupIndex, 427, scatterSeed) * 3.0f);
            int placedBins = 0;
            for (int i = 0; i < bins; ++i) {
                float a = yaw + static_cast<float>(i) * kPi * 2.0f / static_cast<float>(bins)
                    + (Rand01(groupIndex * 17 + i, 429, scatterSeed) - 0.5f) * 0.72f;
                float radius = clusterRadius * (0.46f + Rand01(groupIndex * 17 + i, 431, scatterSeed) * 0.52f);
                bool tipped = i == 0
                    ? Rand01(groupIndex * 17 + i, 433, scatterSeed) < 0.70f
                    : Rand01(groupIndex * 17 + i, 433, scatterSeed) < 0.46f;
                if (AddTrashBinProp(build, px + std::sin(a) * radius, pz + std::cos(a) * radius,
                        a + (Rand01(groupIndex * 17 + i, 435, scatterSeed) - 0.5f) * 0.80f,
                        tipped, Rand01(groupIndex * 17 + i, 437, scatterSeed))) {
                    ++placedBins;
                }
            }
            if (placedBins == 0 && AddTrashBinProp(build, px, pz, yaw, Rand01(groupIndex, 439, scatterSeed) < 0.55f,
                    Rand01(groupIndex, 441, scatterSeed))) {
                ++placedBins;
            }
            return placedBins > 0;
        }
