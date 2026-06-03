    bool AddWaterLikeCeilingFromWallPlacement(LiquidCanvasBuildContext& build,
                                             LiquidCeilingFootprintReservations& ceilingReservations,
                                             XMFLOAT3 edge,
                                             XMFLOAT3 inward,
                                             float width,
                                             float depth,
                                             float yaw,
                                             float seed,
                                             float rawSeed,
                                             uint32_t sourceMask,
                                             int scatterSeed,
                                             float tileMin,
                                             float floorReservationPad,
                                             float wallH) {
        const float widthScales[] = {1.40f, 1.22f, 1.0f, 0.86f, 0.72f, 0.60f};
        const float depthScales[] = {3.65f, 3.05f, 2.46f, 1.94f, 1.54f, 1.24f, 1.0f};
        LiquidWallProjectionFit fit{};
        if (!FindLiquidWallProjectionFit(edge, inward, width, depth, yaw, 0.010f, tileMin,
                widthScales, sizeof(widthScales) / sizeof(widthScales[0]),
                depthScales, sizeof(depthScales) / sizeof(depthScales[0]), fit)) {
            return false;
        }
        return AddLiquidCeilingOverlayPlacement(build, ceilingReservations, fit.center.x, fit.center.z,
            fit.width, fit.depth, yaw, seed, rawSeed, sourceMask, fit.source.x, fit.source.z,
            scatterSeed, tileMin, floorReservationPad, wallH, true);
    }
