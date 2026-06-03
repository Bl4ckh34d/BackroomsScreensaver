    void AddLiquidCeilingDripFloorResponse(LiquidCanvasBuildContext& build,
                                           float px,
                                           float pz,
                                           float width,
                                           float depth,
                                           float yaw,
                                           float seed,
                                           int scatterSeed,
                                           float tileMin,
                                           float floorReservationPad,
                                           float wallH,
                                           bool waterLiquid) {
        if (!waterLiquid) return;
        float floorW = width * (0.46f + Rand01(static_cast<int>(seed * 10000.0f), 817, scatterSeed) * 0.18f);
        float floorD = depth * (0.46f + Rand01(static_cast<int>(seed * 10000.0f), 823, scatterSeed) * 0.18f);
        if (!FootprintFitsMaze(px, pz, floorW, floorD, yaw, floorReservationPad, tileMin)) return;
        MarkLiquidCanvasArea(build, px, pz, floorW, floorD, yaw,
            true, false, 0u, true, seed + 0.19f, std::max(floorW, floorD), true, px, pz);
        MarkWetFootstepArea(px, pz, floorW, floorD, yaw, 0.01f, 9.0f);
        MarkWetCeilingDripEmitter({px, 0.10f, pz}, seed);
        AddLiquidScarePoint({px, 0.10f, pz}, std::max(floorW, floorD), wallH, waterLiquid);
    }
