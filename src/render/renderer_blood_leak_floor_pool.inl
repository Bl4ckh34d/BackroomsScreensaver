        float poolW = std::max(leakW, wallSpan * 0.985f);
        float poolD = axisLength * (0.86f + Rand01(leakIndex, 727, scatterSeed) * 0.12f);
        if (canSpreadForward) {
            poolD += axisLength * (0.18f + Rand01(leakIndex, 729, scatterSeed) * 0.24f);
        }
        const float bloodWidthScales[] = {1.08f, 1.0f, 0.94f, 0.86f, 0.72f, 0.60f};
        const float bloodDepthScales[] = {3.05f, 2.54f, 2.04f, 1.60f, 1.28f, 1.0f};
        bool foundBloodFloorFit = false;
        for (float dw : bloodWidthScales) {
            for (float dd : bloodDepthScales) {
                float candidateW = poolW * dw;
                float candidateD = poolD * dd;
                XMFLOAT3 candidateCenter = Add3({wallFrame.center.x, 0.0f, wallFrame.center.z},
                    Scale3(wallFrame.inward, candidateD * 0.5f + 0.006f));
                if (!FootprintFitsMaze(candidateCenter.x, candidateCenter.z, candidateW, candidateD,
                        LiquidCardYawForSide(side), floorReservationPad, tileMin)) {
                    continue;
                }
                poolW = candidateW;
                poolD = candidateD;
                foundBloodFloorFit = true;
                break;
            }
            if (foundBloodFloorFit) break;
        }
        XMFLOAT3 floorCenter = Add3({wallFrame.center.x, 0.0f, wallFrame.center.z},
            Scale3(wallFrame.inward, poolD * 0.5f + 0.006f));
        if (addSeamCards) {
            addFloorSeamCard(poolW, poolD, sourceMat);
            MarkWetFootstepArea(floorCenter.x, floorCenter.z, poolW, poolD, LiquidCardYawForSide(side), 0.02f,
                waterLiquid ? 7.5f : 0.0f);
            MarkWetCeilingDripEmitter({floorCenter.x, 0.10f, floorCenter.z}, seed);
            if (canSpreadForward) {
                AddWaterFloorBorderContinuationPlacement(liquidBuild, tile, side, wallFrame.center, wallFrame.right, wallFrame.inward,
                    poolW, poolD, sourceMat, seed, leakIndex + 41000, scatterSeed, tileMin, waterLiquid);
            }
        }
        if (wallOnly || gBloodDebugEveryWall) return true;
