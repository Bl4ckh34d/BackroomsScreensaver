    bool AddWaterFloorBorderContinuationPlacement(LiquidCanvasBuildContext& build,
                                                  Tile tile,
                                                  int side,
                                                  const XMFLOAT3& wallCenter,
                                                  const XMFLOAT3& right,
                                                  const XMFLOAT3& inward,
                                                  float width,
                                                  float sourceDepth,
                                                  float material,
                                                  float seed,
                                                  int tag,
                                                  int scatterSeed,
                                                  float tileMin,
                                                  bool waterLiquid) {
        (void)sourceDepth;
        (void)material;
        Tile nextTile = NeighborForMazeSide(tile, OppositeMazeSide(side));
        if (!RenderMazeView().IsOpen(nextTile.x, nextTile.y)) return false;
        float axisLength = (side == 0 || side == 1) ? build.surface.tileD : build.surface.tileW;
        float crossLength = (side == 0 || side == 1) ? build.surface.tileW : build.surface.tileD;
        float continuationDepth = axisLength * (0.54f + Rand01(tag, 1511, scatterSeed) * 0.38f);
        float continuationWidth = std::min(crossLength * 0.98f, width * (1.06f + Rand01(tag, 1517, scatterSeed) * 0.20f));
        float startDistance = std::max(0.04f, axisLength - 0.020f);
        XMFLOAT3 nearCenter = Add3({wallCenter.x, kBloodFloorDecalLift, wallCenter.z},
            Scale3(inward, startDistance));
        nearCenter = Add3(nearCenter, Scale3(right, (Rand01(tag, 1523, scatterSeed) - 0.5f) * width * 0.10f));
        XMFLOAT3 farCenter = Add3(nearCenter, Scale3(inward, continuationDepth));
        float cx = (nearCenter.x + farCenter.x) * 0.5f;
        float cz = (nearCenter.z + farCenter.z) * 0.5f;
        float yaw = ForwardYawForMazeSide(side);
        if (!FootprintFitsMaze(cx, cz, continuationWidth, continuationDepth, yaw, 0.010f, tileMin)) {
            return false;
        }
        MarkLiquidCanvasArea(build, cx, cz, continuationWidth, continuationDepth, yaw, waterLiquid,
            false, 1u << static_cast<uint32_t>(side), false, seed + 0.41f,
            std::max(continuationWidth, continuationDepth), true, nearCenter.x, nearCenter.z);
        MarkWetFootstepArea(cx, cz, continuationWidth, continuationDepth, yaw, 0.02f, 7.5f);
        return true;
    }
