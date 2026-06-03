    bool AddWaterLikeCeilingPlacement(LiquidCanvasBuildContext& build,
                                      LiquidCeilingFootprintReservations& ceilingReservations,
                                      float px,
                                      float pz,
                                      float width,
                                      float depth,
                                      float yaw,
                                      float seed,
                                      float rawSeed,
                                      int scatterSeed,
                                      float tileMin,
                                      float floorReservationPad,
                                      float wallH) {
        float widthScale = 1.0f;
        float depthScale = 1.0f;
        const float widthScales[] = {2.28f, 1.92f, 1.58f, 1.30f, 1.0f, 0.86f};
        const float depthScales[] = {3.45f, 2.85f, 2.26f, 1.76f, 1.36f, 1.0f};
        bool found = FindLiquidFootprintScale(px, pz, width, depth, yaw, 0.010f, tileMin,
            widthScales, sizeof(widthScales) / sizeof(widthScales[0]),
            depthScales, sizeof(depthScales) / sizeof(depthScales[0]),
            widthScale, depthScale);
        if (!found) {
            const float narrowWidthScales[] = {0.78f, 0.66f, 0.54f};
            found = FindLiquidFootprintScale(px, pz, width, depth, yaw, 0.010f, tileMin,
                narrowWidthScales, sizeof(narrowWidthScales) / sizeof(narrowWidthScales[0]),
                depthScales, sizeof(depthScales) / sizeof(depthScales[0]),
                widthScale, depthScale);
        }
        if (!found) return false;
        return AddLiquidCeilingOverlayPlacement(build, ceilingReservations, px, pz,
            width * widthScale, depth * depthScale, yaw, seed, rawSeed,
            0u, std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN(),
            scatterSeed, tileMin, floorReservationPad, wallH, true);
    }
