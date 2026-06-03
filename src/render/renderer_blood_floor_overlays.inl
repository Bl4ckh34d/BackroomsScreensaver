// Blood damage placement overlays.

    bool AddBloodFloorOverlayPlacement(LiquidCanvasBuildContext& build,
                                       std::vector<FloorFootprint>& floorReservations,
                                       float px,
                                       float pz,
                                       float width,
                                       float depth,
                                       float yaw,
                                       float seed,
                                       float layerLift,
                                       float tileMin,
                                       float floorReservationPad,
                                       float wallH,
                                       bool waterLiquid) {
        (void)layerLift;
        if (!LongFloorFootprintClear(floorReservations, px, pz, width, depth, yaw, tileMin, floorReservationPad)) {
            width *= 0.62f;
            depth *= 0.62f;
            if (!FloorFootprintClear(floorReservations, px, pz, width, depth, yaw, tileMin, floorReservationPad)) {
                return false;
            }
        }
        if (!ReserveFloorFootprint(floorReservations, px, pz, width, depth, yaw, tileMin, floorReservationPad)) return false;
        MarkLiquidCanvasArea(build, px, pz, width, depth, yaw, waterLiquid, false, 0u, true, seed, std::max(width, depth));
        MarkWetFootstepArea(px, pz, width, depth, yaw);
        AddLiquidScarePoint({px, 0.10f, pz}, std::max(width, depth), wallH, waterLiquid);
        return true;
    }
