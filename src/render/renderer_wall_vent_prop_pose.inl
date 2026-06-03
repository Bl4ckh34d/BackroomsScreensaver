
        const float wallH = ctx.wallH;
        XMFLOAT3 c = RenderMazeView().WorldCenter(t, 0.0f);
        float wallSpan = (side == 0 || side == 1) ? ctx.tileW : ctx.tileD;
        if (wallSpan < 1.38f || wallH < 1.12f) return;
        float lateralLimit = std::max(0.0f, wallSpan * 0.5f - 0.68f);
        float lateral = (PropPlacementTileHash(t.x, t.y, 41.0f + seed) - 0.5f) * lateralLimit * 2.0f;
        bool lowVent = PropPlacementTileHash(t.x, t.y, 42.2f + seed) < 0.34f;
        float ventW = lowVent ? 0.68f : 0.92f;
        float ventH = lowVent ? 0.24f : 0.34f;
        constexpr float kLowVentFloorGap = 0.085f;
        constexpr float kHighVentCeilingGap = 0.095f;
        constexpr float kVentVerticalClearance = 0.055f;
        float minVentY = ventH * 0.5f + kVentVerticalClearance;
        float maxVentY = std::max(minVentY, wallH - ventH * 0.5f - kVentVerticalClearance);
        float yCenter = lowVent
            ? kLowVentFloorGap + ventH * 0.5f
            : wallH - kHighVentCeilingGap - ventH * 0.5f;
        yCenter = std::clamp(yCenter, minVentY, maxVentY);
        float yaw = 0.0f;
        XMFLOAT3 center{c.x, yCenter, c.z};
        if (side == 0) {
            yaw = 0.0f;
            center = {c.x + lateral, yCenter, c.z - ctx.tileD * 0.5f + 0.018f};
        } else if (side == 1) {
            yaw = kPi;
            center = {c.x + lateral, yCenter, c.z + ctx.tileD * 0.5f - 0.018f};
        } else if (side == 2) {
            yaw = kPi * 0.5f;
            center = {c.x - ctx.tileW * 0.5f + 0.018f, yCenter, c.z + lateral};
        } else {
            yaw = -kPi * 0.5f;
            center = {c.x + ctx.tileW * 0.5f - 0.018f, yCenter, c.z + lateral};
        }
