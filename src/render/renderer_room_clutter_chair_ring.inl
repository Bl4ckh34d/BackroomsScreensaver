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
