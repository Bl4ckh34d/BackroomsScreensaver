    bool AddWaterLikeFloorPlacement(LiquidCanvasBuildContext& build,
                                    std::vector<FloorFootprint>& floorReservations,
                                    float px,
                                    float pz,
                                    float width,
                                    float depth,
                                    float yaw,
                                    float seed,
                                    float layerLift,
                                    float rawSeed,
                                    bool downstream,
                                    float tileMin,
                                    float floorReservationPad,
                                    float wallH) {
        float widthScale = 1.0f;
        float depthScale = 1.0f;
        const float widthScales[] = {2.18f, 1.86f, 1.56f, 1.28f, 1.0f, 0.86f};
        const float depthScales[] = {3.35f, 2.75f, 2.20f, 1.72f, 1.36f, 1.0f};
        bool found = FindLiquidFootprintScale(px, pz, width, depth, yaw, floorReservationPad, tileMin,
            widthScales, sizeof(widthScales) / sizeof(widthScales[0]),
            depthScales, sizeof(depthScales) / sizeof(depthScales[0]),
            widthScale, depthScale);
        if (!found) {
            const float narrowWidthScales[] = {0.78f, 0.66f, 0.54f};
            found = FindLiquidFootprintScale(px, pz, width, depth, yaw, floorReservationPad, tileMin,
                narrowWidthScales, sizeof(narrowWidthScales) / sizeof(narrowWidthScales[0]),
                depthScales, sizeof(depthScales) / sizeof(depthScales[0]),
                widthScale, depthScale);
        }
        if (!found) return false;
        return AddLiquidFloorOverlayPlacement(build, floorReservations, px, pz,
            width * widthScale, depth * depthScale, yaw, seed, 0.010f + layerLift, rawSeed,
            0u, px, pz, downstream, tileMin, floorReservationPad, wallH, true);
    }
