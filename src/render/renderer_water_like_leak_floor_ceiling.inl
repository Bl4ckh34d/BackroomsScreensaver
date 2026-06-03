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
