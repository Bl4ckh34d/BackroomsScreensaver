        float axisLength = (side == 0 || side == 1) ? liquidBuild.surface.tileD : liquidBuild.surface.tileW;
        Tile forwardTile = NeighborForMazeSide(tile, OppositeMazeSide(side));
        bool canSpreadForward = RenderMazeView().IsOpen(forwardTile.x, forwardTile.y);
        float sourceD = axisLength * (0.88f + Rand01(leakIndex, 719, scatterSeed) * 0.10f);
        if (canSpreadForward) {
            sourceD += axisLength * (0.16f + Rand01(leakIndex, 721, scatterSeed) * 0.22f);
        }
        if (addSeamCards) {
            addCeilingSeamCard(sourceD, sourceMat);
            if (!waterLiquid) {
                float seamYaw = ForwardYawForMazeSide(side);
                XMFLOAT3 seamWallEdge = Add3({wallFrame.center.x, liquidBuild.wallH - kBloodCeilingDecalInset, wallFrame.center.z},
                    Scale3(wallFrame.inward, -kBloodLeakWallDecalInset));
                XMFLOAT3 seamCenter = Add3(seamWallEdge, Scale3(wallFrame.inward, sourceD * 0.5f + kBloodLeakSeamInset));
                AddBloodCeilingPropagationPlacement(waterBuild, liquidBuild, floorReservations, ceilingReservations,
                    tile, seamCenter.x, seamCenter.z, leakW, sourceD, seamYaw, seed,
                    leakIndex, side, scatterSeed, tileMin, floorReservationPad, waterLiquid);
            }
        }

