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
