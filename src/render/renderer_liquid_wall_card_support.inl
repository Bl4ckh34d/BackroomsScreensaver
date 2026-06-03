        XMFLOAT3 tileCenter = RenderMazeView().WorldCenter(tile, 0.0f);
        float minAlong = 0.0f;
        float maxAlong = 0.0f;
        if (!WallWaterSupportSpan(waterBuild, tile, side, minAlong, maxAlong)) return false;

        constexpr float kBloodWallDecalInset = 0.0050f;
        constexpr float wallDecalMargin = 0.13f;
        minAlong += wallDecalMargin;
        maxAlong -= wallDecalMargin;
        float available = maxAlong - minAlong;
        if (available < 0.28f) return false;

        width = std::min(width, available);
        float halfWidth = width * 0.5f;
        float desiredAlong = (side == 0 || side == 1) ? tileCenter.x + lateral : tileCenter.z + lateral;
        float clampedAlong = std::clamp(desiredAlong, minAlong + halfWidth, maxAlong - halfWidth);
        float clampedLateral = (side == 0 || side == 1) ? clampedAlong - tileCenter.x : clampedAlong - tileCenter.z;

        constexpr float wallBloodFloorMargin = 0.002f;
        constexpr float wallBloodCeilingMargin = 0.004f;
        height = liquidBuild.wallH - wallBloodFloorMargin - wallBloodCeilingMargin;
        float centerY = wallBloodFloorMargin + height * 0.5f;
