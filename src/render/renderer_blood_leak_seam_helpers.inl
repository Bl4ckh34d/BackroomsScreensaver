        float bloodQuality = std::clamp(settingsRuntime_.live.bloodShaderQuality, 0.25f, 1.0f);
        bool addSeamCards = waterLiquid || !wallOnly || gBloodDebugEveryWall || bloodQuality >= 0.52f ||
            Rand01(leakIndex, 741, scatterSeed) < bloodQuality * 0.75f;
        auto addCeilingSeamCard = [&](float depth, float material) {
            (void)material;
            float seamY = NextLiquidCeilingY(coverage, wallFrame.center.x, wallFrame.center.z, liquidBuild.wallH - kBloodCeilingDecalInset);
            XMFLOAT3 seamWallEdge = Add3({wallFrame.center.x, seamY, wallFrame.center.z},
                Scale3(wallFrame.inward, -kBloodLeakWallDecalInset));
            XMFLOAT3 seamCenter = Add3(seamWallEdge, Scale3(wallFrame.inward, kBloodLeakSeamInset));
            XMFLOAT3 ceilingCenter = Add3(seamCenter, Scale3(wallFrame.inward, depth * 0.5f));
            MarkLiquidCanvasArea(liquidBuild, ceilingCenter.x, ceilingCenter.z, leakW, depth, LiquidCardYawForSide(side),
                waterLiquid, true, 1u << static_cast<uint32_t>(side), false, seed + 0.37f, depth,
                false, seamCenter.x, seamCenter.z);
        };

        auto addFloorSeamCard = [&](float width, float depth, float material) {
            XMFLOAT3 seamWallEdge = Add3({wallFrame.center.x, kBloodFloorDecalLift, wallFrame.center.z},
                Scale3(wallFrame.inward, -kBloodLeakWallDecalInset));
            XMFLOAT3 seamCenter = Add3(seamWallEdge, Scale3(wallFrame.inward, kBloodLeakSeamInset));
            XMFLOAT3 floorCenter = Add3(seamCenter, Scale3(wallFrame.inward, depth * 0.5f));
            QueueLiquidFloorSeam(pendingFloorSeams, tile, side, floorCenter.x, floorCenter.z, width, depth,
                LiquidCardYawForSide(side), material, seamCenter.x, seamCenter.z, waterLiquid);
        };

