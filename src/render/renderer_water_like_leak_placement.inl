    bool AddWaterLikeLeakPlacement(LiquidCanvasBuildContext& build,
                                   std::vector<FloorFootprint>& floorReservations,
                                   LiquidCeilingFootprintReservations& ceilingReservations,
                                   LiquidDamageCoverage& coverage,
                                   Tile tile,
                                   int side,
                                   int leakIndex,
                                   bool wallOnly,
                                   int scatterSeed,
                                   float tileW,
                                   float tileD,
                                   float tileMin,
                                   float floorReservationPad,
                                   float wallH) {
        if (!WallHasWaterSurface(tile, side)) return false;
        if (LiquidDamageTileBlocked(coverage, tile) || LiquidCenterSeepCovered(coverage, tile)) {
            return false;
        }
        XMFLOAT3 c = RenderMazeView().WorldCenter(tile, 0.0f);
        float seed = Rand01(leakIndex, 2401, scatterSeed);
        float wallSpan = (side == 0 || side == 1) ? tileW : tileD;
        float leakW = wallSpan * (0.82f + Rand01(leakIndex, 2407, scatterSeed) * 0.13f);
        float lateralLimit = std::max(0.0f, wallSpan * 0.5f - leakW * 0.5f - 0.16f);
        float lateral = (Rand01(leakIndex, 2409, scatterSeed) - 0.5f) * lateralLimit * 2.0f;
        float topY = wallH - 0.0015f;
        float bottomY = 0.0015f;
        float height = std::max(0.2f, topY - bottomY);
        float centerY = (topY + bottomY) * 0.5f;
        constexpr float kWaterLikeWallDecalInset = 0.0045f;
        LiquidWallLeakFrame wallFrame = BuildLiquidWallLeakFrame(build.surface, c, side, lateral,
            centerY, leakW, height, kWaterLikeWallDecalInset, seed);
        float sourceMat = WaterLikeSurfaceMaterial(seed, 0.965f + seed * 0.025f);
        AddQuadUV(build.vertices, build.liquidIndices,
            wallFrame.a, wallFrame.b, wallFrame.c, wallFrame.d, wallFrame.normal, wallFrame.right,
            {0, 1}, {1, 1}, {1, 0}, {0, 0}, sourceMat);

        float axisLength = (side == 0 || side == 1) ? tileD : tileW;
        Tile forwardTile = NeighborForMazeSide(tile, OppositeMazeSide(side));
        bool canSpreadForward = RenderMazeView().IsOpen(forwardTile.x, forwardTile.y);
        float sourceD = axisLength * (0.88f + Rand01(leakIndex, 2419, scatterSeed) * 0.10f);
        if (canSpreadForward) sourceD += axisLength * (0.16f + Rand01(leakIndex, 2421, scatterSeed) * 0.22f);
        float seamYaw = LiquidCardYawForSide(side);
        XMFLOAT3 ceilingEdge = Add3({wallFrame.center.x, wallH - kBloodCeilingDecalInset, wallFrame.center.z},
            Scale3(wallFrame.inward, -kWaterLikeWallDecalInset));
        uint32_t sourceMask = 1u << static_cast<uint32_t>(side);
        bool ceilingPlaced = AddWaterLikeCeilingFromWallPlacement(build, ceilingReservations,
            ceilingEdge, wallFrame.inward, leakW, sourceD, seamYaw, seed, 0.965f + seed * 0.025f,
            sourceMask, scatterSeed, tileMin, floorReservationPad, wallH);
        float poolD = axisLength * (0.86f + Rand01(leakIndex, 2427, scatterSeed) * 0.12f);
        if (canSpreadForward) poolD += axisLength * (0.18f + Rand01(leakIndex, 2429, scatterSeed) * 0.24f);
        XMFLOAT3 floorEdge{wallFrame.center.x, 0.0f, wallFrame.center.z};
        bool floorPlaced = AddWaterLikeFloorFromWallPlacement(build, floorReservations,
            floorEdge, wallFrame.inward, leakW, poolD, seamYaw, seed, 0.0f, 0.965f + seed * 0.025f,
            sourceMask, tileMin, floorReservationPad, wallH);
        XMFLOAT3 floorCenter = Add3(floorEdge, Scale3(wallFrame.inward, poolD * 0.5f + 0.006f));
        if (floorPlaced && canSpreadForward) {
            AddWaterFloorBorderContinuationPlacement(build, tile, side, wallFrame.center, wallFrame.right, wallFrame.inward,
                leakW, poolD, sourceMat, seed, leakIndex + 43000, scatterSeed, tileMin, true);
        }
        if (ceilingPlaced || floorPlaced) {
            ReserveLiquidDamageTile(coverage, tile);
            ReserveLiquidDamageCoveredTile(coverage, tile);
            if (canSpreadForward) ReserveLiquidDamageCoveredTile(coverage, forwardTile);
        }
        if (ceilingPlaced && floorPlaced) {
            MarkWetCeilingDripEmitter({floorCenter.x, 0.10f, floorCenter.z}, seed);
        }

        if (!wallOnly && !gEffectDebugViewer) {
            BloodScarePoint scare{};
            scare.pos = {floorCenter.x, 0.10f, floorCenter.z};
            scare.source = {wallFrame.center.x, wallH - 0.035f, wallFrame.center.z};
            scare.normal = wallFrame.normal;
            scare.radius = std::clamp(1.65f + std::max(leakW, poolD) * 0.70f, 2.10f, 4.80f);
            scare.focusDelaySeconds = 0.22f + Rand01(leakIndex, 2433, scatterSeed) * 0.44f;
            scare.requireFacing = true;
            scare.dreadScale = 0.42f;
            scare.revealBlood = false;
            scareRuntime_.bloodScarePoints.push_back(scare);
        }
        return true;
    }
