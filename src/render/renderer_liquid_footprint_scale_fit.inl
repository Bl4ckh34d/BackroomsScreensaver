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
