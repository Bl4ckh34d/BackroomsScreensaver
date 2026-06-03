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
