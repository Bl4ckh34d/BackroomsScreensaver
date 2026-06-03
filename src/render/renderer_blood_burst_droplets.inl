        int droplets = 24 + static_cast<int>(Rand01(burstIndex, 709, scatterSeed) * 42.0f);
        for (int d = 0; d < droplets; ++d) {
            float dyaw = Rand01(burstIndex * 31 + d, 719, scatterSeed) * kPi * 2.0f;
            float radius = std::pow(Rand01(burstIndex * 31 + d, 727, scatterSeed), 0.56f) * tileAvg * 1.54f;
            float dx = std::cos(dyaw) * radius;
            float dz = std::sin(dyaw) * radius;
            float tiny = std::pow(Rand01(burstIndex * 31 + d, 733, scatterSeed), 2.45f);
            float dw = 0.028f + tiny * 0.20f;
            float dd = 0.024f + std::pow(Rand01(burstIndex * 31 + d, 739, scatterSeed), 2.35f) * 0.18f;
            AddBloodFloorOverlayPlacement(liquidBuild, floorReservations, px + dx, pz + dz, dw, dd, dyaw,
                Rand01(burstIndex * 31 + d, 743, scatterSeed),
                static_cast<float>(d) * kBloodFloorDecalLayerStep,
                tileMin, floorReservationPad, liquidBuild.wallH, waterLiquid);
        }
