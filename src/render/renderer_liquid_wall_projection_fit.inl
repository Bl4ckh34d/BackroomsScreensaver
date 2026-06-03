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
