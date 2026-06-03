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
