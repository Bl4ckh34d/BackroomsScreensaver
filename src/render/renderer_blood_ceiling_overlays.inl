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

    bool AddLiquidCeilingOverlayPlacement(LiquidCanvasBuildContext& build,
                                          LiquidCeilingFootprintReservations& ceilingReservations,
                                          float px,
                                          float pz,
                                          float width,
                                          float depth,
                                          float yaw,
                                          float seed,
                                          float rawSeed,
                                          uint32_t sourceMask,
                                          float sourceX,
                                          float sourceZ,
                                          int scatterSeed,
                                          float tileMin,
                                          float floorReservationPad,
                                          float wallH,
                                          bool waterLiquid) {
        if (!ReserveLiquidCeilingFootprint(ceilingReservations, px, pz, width, depth, yaw, tileMin, 0.010f, 0.002f)) {
            return false;
        }
        bool fromWall = rawSeed >= 0.96f || sourceMask != 0u;
        MarkLiquidCanvasArea(build, px, pz, width, depth, yaw, waterLiquid, true, sourceMask, !fromWall,
            seed, std::max(width, depth), false, sourceX, sourceZ);
        AddLiquidCeilingDripFloorResponse(build, px, pz, width, depth, yaw, seed,
            scatterSeed, tileMin, floorReservationPad, wallH, waterLiquid);
        AddLiquidScarePoint({px, wallH - 0.08f, pz}, std::max(width, depth), wallH, waterLiquid);
        return true;
    }

    bool AddLiquidFloorOverlayPlacement(LiquidCanvasBuildContext& build,
                                        std::vector<FloorFootprint>& floorReservations,
                                        float px,
                                        float pz,
                                        float width,
                                        float depth,
                                        float yaw,
                                        float seed,
                                        float layerLift,
                                        float rawSeed,
                                        uint32_t sourceMask,
                                        float sourceX,
                                        float sourceZ,
                                        bool downstream,
                                        float tileMin,
                                        float floorReservationPad,
                                        float wallH,
                                        bool waterLiquid) {
        (void)layerLift;
        if (!FootprintFitsMaze(px, pz, width, depth, yaw, floorReservationPad, tileMin)) return false;
        floorReservations.push_back(MakeFloorFootprint(px, pz, width, depth, yaw, 0.002f));
        bool fromWall = rawSeed >= 0.96f || sourceMask != 0u;
        MarkLiquidCanvasArea(build, px, pz, width, depth, yaw, waterLiquid, false, sourceMask, !fromWall,
            seed, std::max(width, depth), downstream, sourceX, sourceZ);
        MarkWetFootstepArea(px, pz, width, depth, yaw);
        AddLiquidScarePoint({px, 0.10f, pz}, std::max(width, depth), wallH, waterLiquid);
        return true;
    }

    bool AddBloodCeilingFloorPairPlacement(LiquidCanvasBuildContext& build,
                                           std::vector<FloorFootprint>& floorReservations,
                                           LiquidCeilingFootprintReservations& ceilingReservations,
                                           float px,
                                           float pz,
                                           float width,
                                           float depth,
                                           float yaw,
                                           float seed,
                                           float floorScale,
                                           float tileMin,
                                           float floorReservationPad,
                                           float wallH,
                                           bool waterLiquid) {
        float floorW = width * floorScale;
        float floorD = depth * floorScale;
        if (!FloorFootprintClear(floorReservations, px, pz, floorW, floorD, yaw, tileMin, floorReservationPad)) return false;
        if (!LiquidCeilingFootprintClear(ceilingReservations, px, pz, width, depth, yaw, tileMin, 0.010f)) return false;
        floorReservations.push_back(MakeFloorFootprint(px, pz, floorW, floorD, yaw, floorReservationPad));
        AddLiquidCeilingFootprintReservation(ceilingReservations, px, pz, width, depth, yaw, 0.010f);
        MarkLiquidCanvasArea(build, px, pz, floorW, floorD, yaw, waterLiquid, false, 0u, true,
            seed, std::max(floorW, floorD), true, px, pz);
        MarkWetFootstepArea(px, pz, floorW, floorD, yaw);
        MarkWetCeilingDripEmitter({px, 0.10f, pz}, seed);
        MarkLiquidCanvasArea(build, px, pz, width, depth, yaw, waterLiquid, true, 0u, true, seed, std::max(width, depth));
        AddLiquidScarePoint({px, 0.10f, pz}, std::max(floorW, floorD), wallH, waterLiquid);
        AddLiquidScarePoint({px, wallH - 0.08f, pz}, std::max(width, depth), wallH, waterLiquid);
        return true;
    }
