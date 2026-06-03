        XMFLOAT3 c = RenderMazeView().WorldCenter(tile, 0.0f);
        float r0 = Rand01(burstIndex, 601, scatterSeed);
        float r1 = Rand01(burstIndex, 607, scatterSeed);
        float r2 = Rand01(burstIndex, 613, scatterSeed);
        float yaw = r2 * kPi * 2.0f;
        float px = c.x + (r0 - 0.5f) * liquidBuild.surface.tileW * 0.48f;
        float pz = c.z + (r1 - 0.5f) * liquidBuild.surface.tileD * 0.48f;
        float base = 0.62f + Rand01(burstIndex, 617, scatterSeed) * 0.98f;
        AddBloodFloorOverlayPlacement(liquidBuild, floorReservations, px, pz,
            base * (0.96f + Rand01(burstIndex, 619, scatterSeed) * 0.74f),
            base * (0.58f + Rand01(burstIndex, 623, scatterSeed) * 0.70f),
            yaw, r0, 0.0f, tileMin, floorReservationPad, liquidBuild.wallH, waterLiquid);
        AddBloodFloorNeighborSpillPlacement(liquidBuild, floorReservations, tile, px, pz, r0,
            burstIndex, std::clamp(base / std::max(0.01f, tileAvg), 0.48f, 1.08f),
            scatterSeed, tileMin, floorReservationPad, waterLiquid);
        if (Rand01(burstIndex, 631, scatterSeed) < 0.78f) {
            float ceilingX = px + (Rand01(burstIndex, 641, scatterSeed) - 0.5f) * 0.45f;
            float ceilingZ = pz + (Rand01(burstIndex, 643, scatterSeed) - 0.5f) * 0.45f;
            float ceilingW = base * (0.78f + Rand01(burstIndex, 647, scatterSeed) * 0.86f);
            float ceilingD = base * (0.64f + Rand01(burstIndex, 653, scatterSeed) * 0.78f);
            float ceilingYaw = yaw + Rand01(burstIndex, 659, scatterSeed) * 0.9f;
            if (AddBloodCeilingFloorPairPlacement(liquidBuild, floorReservations, ceilingReservations,
                    ceilingX, ceilingZ, ceilingW, ceilingD, ceilingYaw, r1, 0.86f,
                    tileMin, floorReservationPad, liquidBuild.wallH, waterLiquid)) {
                AddBloodCeilingPropagationPlacement(waterBuild, liquidBuild, floorReservations, ceilingReservations,
                    tile, ceilingX, ceilingZ, ceilingW, ceilingD, ceilingYaw, r1,
                    burstIndex, -1, scatterSeed, tileMin, floorReservationPad, waterLiquid);
            }
        }

