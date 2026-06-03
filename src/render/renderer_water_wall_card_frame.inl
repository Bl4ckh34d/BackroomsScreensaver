
        const float tileW = build.surface.tileW;
        const float tileD = build.surface.tileD;
        const float wallH = build.wallH;
        XMFLOAT3 c = RenderMazeView().WorldCenter(t, 0.0f);
        XMFLOAT3 normal{0.0f, 0.0f, 1.0f};
        XMFLOAT3 right{1.0f, 0.0f, 0.0f};
        XMFLOAT3 center{c.x, yCenter, c.z};
        constexpr float kWaterWallDecalInset = 0.0045f;
        float minAlong = 0.0f;
        float maxAlong = 0.0f;
        if (!WallWaterSupportSpan(build, t, side, minAlong, maxAlong)) return false;
        constexpr float wallDecalMargin = 0.10f;
        minAlong += wallDecalMargin;
        maxAlong -= wallDecalMargin;
        float available = maxAlong - minAlong;
        if (available < 0.24f) return false;
        w = std::min(w, available);
        float halfW = w * 0.5f;
        float desiredAlong = (side == 0 || side == 1) ? c.x + lateral : c.z + lateral;
        float clampedAlong = std::clamp(desiredAlong, minAlong + halfW, maxAlong - halfW);
        if (side == 0) {
            normal = {0.0f, 0.0f, 1.0f};
            right = {1.0f, 0.0f, 0.0f};
            center = {clampedAlong, yCenter, c.z - tileD * 0.5f + kWaterWallDecalInset};
        } else if (side == 1) {
            normal = {0.0f, 0.0f, -1.0f};
            right = {-1.0f, 0.0f, 0.0f};
            center = {clampedAlong, yCenter, c.z + tileD * 0.5f - kWaterWallDecalInset};
        } else if (side == 2) {
            normal = {1.0f, 0.0f, 0.0f};
            right = {0.0f, 0.0f, 1.0f};
            center = {c.x - tileW * 0.5f + kWaterWallDecalInset, yCenter, clampedAlong};
        } else {
            normal = {-1.0f, 0.0f, 0.0f};
            right = {0.0f, 0.0f, -1.0f};
            center = {c.x + tileW * 0.5f - kWaterWallDecalInset, yCenter, clampedAlong};
        }
        h = sourceFromCeiling
            ? wallH - 0.003f
            : std::clamp(h, 0.08f, wallH - 0.003f);
        center.y = sourceFromCeiling
            ? wallH - 0.0015f - h * 0.5f
            : 0.0015f + h * 0.5f;
