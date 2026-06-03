// Water-like liquid damage placement helpers.
// Included inside Renderer private section after shared liquid placement helpers.

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
