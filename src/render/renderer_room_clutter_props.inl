    bool AddRoomClutterGroupProp(StaticPropPlacementBuildContext& build, Tile t, int groupIndex, uint32_t scatterSeed) {
        if (!IsRoomLike(t)) return false;
        XMFLOAT3 c = RenderMazeView().WorldCenter(t, 0.0f);
        const float tileW = build.surface.tileW;
        const float tileD = build.surface.tileD;
        float yaw = Rand01(groupIndex, 311, scatterSeed) * kPi * 2.0f;
        float kind = Rand01(groupIndex, 313, scatterSeed);
        float px = c.x + (Rand01(groupIndex, 317, scatterSeed) - 0.5f) * tileW * 0.44f;
        float pz = c.z + (Rand01(groupIndex, 331, scatterSeed) - 0.5f) * tileD * 0.44f;
        auto longFootprintClear = [&](float cx, float cz, float width, float depth, float cyaw, float pad) {
            return LongFloorFootprintClear(build.floorReservations, cx, cz, width, depth, cyaw, build.tileMin, pad);
        };

        if (kind < 0.38f) {
            float w = 1.10f + Rand01(groupIndex, 337, scatterSeed) * 0.42f;
            float d = 0.68f + Rand01(groupIndex, 347, scatterSeed) * 0.28f;
            float chairRing = std::max(w, d) * 0.5f + 0.74f;
            if (!longFootprintClear(px, pz, chairRing * 2.0f + 1.08f, chairRing * 2.0f + 1.02f, yaw, 0.070f)) return false;
            float tableSeed = Rand01(groupIndex, 349, scatterSeed);
            if (!AddStandingTableProp(build, px, pz, w, d, yaw, tableSeed)) return false;
            if (Rand01(groupIndex, 351, scatterSeed) < 0.64f) {
                AddDeskLampOnSurfaceProp(build, px, pz, 0.745f + tableSeed * 0.05f, yaw, w, d, Rand01(groupIndex, 352, scatterSeed));
            }
            int chairs = 2 + static_cast<int>(Rand01(groupIndex, 353, scatterSeed) * 3.0f);
            int placedChairs = 0;
            for (int i = 0; i < chairs; ++i) {
                float a = yaw + static_cast<float>(i) * kPi * 2.0f / static_cast<float>(chairs) + RandRange(-0.22f, 0.22f);
                float radius = chairRing + Rand01(groupIndex * 7 + i, 359, scatterSeed) * 0.18f;
                XMFLOAT3 chairPos{px + std::sin(a) * radius, 0.0f, pz + std::cos(a) * radius};
                float chairYaw = a + kPi + RandRange(-0.32f, 0.32f);
                bool placed = Rand01(groupIndex * 7 + i, 367, scatterSeed) < 0.22f
                    ? AddTippedChairProp(build, chairPos.x, chairPos.z, chairYaw, false, Rand01(groupIndex * 7 + i, 371, scatterSeed))
                    : AddChairProp(build, chairPos, chairYaw, Rand01(groupIndex * 7 + i, 373, scatterSeed) < 0.42f);
                if (placed) ++placedChairs;
            }
            if (Rand01(groupIndex, 375, scatterSeed) < 0.72f) {
                float binA = yaw + Rand01(groupIndex, 377, scatterSeed) * kPi * 2.0f;
                float binR = chairRing * (0.42f + Rand01(groupIndex, 381, scatterSeed) * 0.28f);
                AddTrashBinProp(build, px + std::sin(binA) * binR, pz + std::cos(binA) * binR,
                    binA + kPi * 0.5f, Rand01(groupIndex, 385, scatterSeed) < 0.38f,
                    Rand01(groupIndex, 387, scatterSeed));
            }
            return true;
        }

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

        int chairs = 3 + static_cast<int>(Rand01(groupIndex, 409, scatterSeed) * 4.0f);
        float ring = 0.88f + Rand01(groupIndex, 407, scatterSeed) * 0.28f;
        if (!longFootprintClear(px, pz, ring * 2.0f + 1.02f, ring * 2.0f + 1.02f, yaw, 0.070f)) return false;
        int placedChairs = 0;
        for (int i = 0; i < chairs; ++i) {
            float a = yaw + static_cast<float>(i) * kPi * 2.0f / static_cast<float>(chairs);
            float radius = ring + Rand01(groupIndex * 11 + i, 419, scatterSeed) * 0.20f;
            float cx = px + std::sin(a) * radius;
            float cz = pz + std::cos(a) * radius;
            float chairYaw = a + RandRange(-0.50f, 0.50f);
            bool placed = Rand01(groupIndex * 11 + i, 421, scatterSeed) < 0.34f
                ? AddTippedChairProp(build, cx, cz, chairYaw, true, Rand01(groupIndex * 11 + i, 431, scatterSeed))
                : AddChairProp(build, {cx, 0.0f, cz}, chairYaw, true);
            if (placed) ++placedChairs;
        }
        if (Rand01(groupIndex, 443, scatterSeed) < 0.58f) {
            AddTrashBinProp(build, px, pz, yaw + Rand01(groupIndex, 445, scatterSeed) * kPi,
                Rand01(groupIndex, 447, scatterSeed) < 0.42f, Rand01(groupIndex, 449, scatterSeed));
        }
        return placedChairs > 0;
    }
