    void AddBloodFloorNeighborSpillPlacement(LiquidCanvasBuildContext& build,
                                             std::vector<FloorFootprint>& floorReservations,
                                             Tile tile,
                                             float sourceX,
                                             float sourceZ,
                                             float seed,
                                             int tag,
                                             float strength,
                                             int scatterSeed,
                                             float tileMin,
                                             float floorReservationPad,
                                             bool waterLiquid) {
        XMFLOAT3 c = RenderMazeView().WorldCenter(tile, 0.0f);
        for (int side = 0; side < 4; ++side) {
            Tile neighbor = NeighborForMazeSide(tile, side);
            if (!RenderMazeView().IsOpen(neighbor.x, neighbor.y)) continue;
            float sideBias = 0.0f;
            if (side == 0) sideBias = (c.z - sourceZ) / std::max(0.001f, build.surface.tileD * 0.5f);
            else if (side == 1) sideBias = (sourceZ - c.z) / std::max(0.001f, build.surface.tileD * 0.5f);
            else if (side == 2) sideBias = (c.x - sourceX) / std::max(0.001f, build.surface.tileW * 0.5f);
            else sideBias = (sourceX - c.x) / std::max(0.001f, build.surface.tileW * 0.5f);
            float chance = std::clamp(0.40f + strength * 0.34f + std::max(0.0f, sideBias) * 0.16f, 0.0f, 0.90f);
            float r0 = Rand01(tag * 19 + side, 1009, scatterSeed);
            if (r0 > chance) continue;
            XMFLOAT3 dir = DirectionForMazeSide(side);
            float axis = side == 0 || side == 1 ? build.surface.tileD : build.surface.tileW;
            float cross = side == 0 || side == 1 ? build.surface.tileW : build.surface.tileD;
            float originalOverlap = axis * (0.16f + Rand01(tag * 19 + side, 1011, scatterSeed) * 0.08f);
            float neighborReach = axis * (0.20f + Rand01(tag * 19 + side, 1013, scatterSeed) * 0.26f) *
                std::clamp(strength, 0.70f, 1.12f);
            float length = originalOverlap + neighborReach;
            float width = cross * (0.24f + Rand01(tag * 19 + side, 1019, scatterSeed) * 0.28f);
            float halfAxis = axis * 0.5f;
            float centerOffset = halfAxis + (neighborReach - originalOverlap) * 0.5f;
            XMFLOAT3 lateralAxis = side == 0 || side == 1
                ? XMFLOAT3{1.0f, 0.0f, 0.0f}
                : XMFLOAT3{0.0f, 0.0f, 1.0f};
            float sourceLateral = (side == 0 || side == 1) ? sourceX - c.x : sourceZ - c.z;
            float lateralLimit = std::max(0.0f, cross - width) * 0.48f;
            float lateral = std::clamp(sourceLateral +
                (Rand01(tag * 19 + side, 1017, scatterSeed) - 0.5f) * lateralLimit * 0.42f,
                -lateralLimit, lateralLimit);
            XMFLOAT3 spillCenter = Add3(c, Add3(Scale3(dir, centerOffset), Scale3(lateralAxis, lateral)));
            float yaw = ForwardYawForMazeSide(side) + (Rand01(tag * 19 + side, 1021, scatterSeed) - 0.5f) * 0.18f;
            AddBloodFloorOverlayPlacement(build, floorReservations, spillCenter.x, spillCenter.z, width, length, yaw,
                std::fmod(seed + 0.17f + static_cast<float>(side) * 0.071f, 0.93f),
                static_cast<float>(side + 1) * kBloodFloorDecalLayerStep * 5.0f,
                tileMin, floorReservationPad, build.wallH, waterLiquid);
        }
    }
