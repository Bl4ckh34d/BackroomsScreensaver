    bool AddWaterLikeFloorFromWallPlacement(LiquidCanvasBuildContext& build,
                                           std::vector<FloorFootprint>& floorReservations,
                                           XMFLOAT3 edge,
                                           XMFLOAT3 inward,
                                           float width,
                                           float depth,
                                           float yaw,
                                           float seed,
                                           float layerLift,
                                           float rawSeed,
                                           uint32_t sourceMask,
                                           float tileMin,
                                           float floorReservationPad,
                                           float wallH) {
        const float widthScales[] = {1.34f, 1.18f, 1.0f, 0.86f, 0.72f, 0.60f};
        const float depthScales[] = {3.85f, 3.20f, 2.62f, 2.08f, 1.62f, 1.28f, 1.0f};
        LiquidWallProjectionFit fit{};
        if (!FindLiquidWallProjectionFit(edge, inward, width, depth, yaw, floorReservationPad, tileMin,
                widthScales, sizeof(widthScales) / sizeof(widthScales[0]),
                depthScales, sizeof(depthScales) / sizeof(depthScales[0]), fit)) {
            return false;
        }
        return AddLiquidFloorOverlayPlacement(build, floorReservations, fit.center.x, fit.center.z,
            fit.width, fit.depth, yaw, seed, 0.010f + layerLift, rawSeed, sourceMask,
            fit.source.x, fit.source.z, false, tileMin, floorReservationPad, wallH, true);
    }
