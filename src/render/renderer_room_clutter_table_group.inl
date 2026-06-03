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
