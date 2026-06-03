// Liquid placement fit helpers.

    bool FindLiquidFootprintScale(float px,
                                  float pz,
                                  float width,
                                  float depth,
                                  float yaw,
                                  float pad,
                                  float tileMin,
                                  const float* widthScales,
                                  size_t widthScaleCount,
                                  const float* depthScales,
                                  size_t depthScaleCount,
                                  float& outWidthScale,
                                  float& outDepthScale) const {
        for (size_t wi = 0; wi < widthScaleCount; ++wi) {
            for (size_t di = 0; di < depthScaleCount; ++di) {
                float widthScale = widthScales[wi];
                float depthScale = depthScales[di];
                if (!FootprintFitsMaze(px, pz, width * widthScale, depth * depthScale, yaw, pad, tileMin)) continue;
                outWidthScale = widthScale;
                outDepthScale = depthScale;
                return true;
            }
        }
        return false;
    }

    bool FindLiquidWallProjectionFit(const XMFLOAT3& edge,
                                     const XMFLOAT3& inward,
                                     float width,
                                     float depth,
                                     float yaw,
                                     float pad,
                                     float tileMin,
                                     const float* widthScales,
                                     size_t widthScaleCount,
                                     const float* depthScales,
                                     size_t depthScaleCount,
                                     LiquidWallProjectionFit& fit) const {
        for (size_t wi = 0; wi < widthScaleCount; ++wi) {
            for (size_t di = 0; di < depthScaleCount; ++di) {
                float candidateWidth = width * widthScales[wi];
                float candidateDepth = depth * depthScales[di];
                XMFLOAT3 center = Add3(edge, Scale3(inward, candidateDepth * 0.5f + 0.010f));
                if (!FootprintFitsMaze(center.x, center.z, candidateWidth, candidateDepth, yaw, pad, tileMin)) continue;
                fit.center = center;
                fit.source = Add3(edge, Scale3(inward, 0.012f));
                fit.width = candidateWidth;
                fit.depth = candidateDepth;
                return true;
            }
        }
        return false;
    }

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
