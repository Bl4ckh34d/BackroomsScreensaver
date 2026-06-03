    void AddWallVentProp(std::vector<Vertex>& vertices,
                         std::vector<uint32_t>& indices,
                         std::vector<Vertex>& instancedVertices,
                         std::vector<uint32_t>& instancedIndices,
                         std::vector<PendingStaticInstance>& pendingStaticInstances,
                         std::vector<InstancedMeshRange>& instancedMeshRanges,
                         const MazeSurfaceBuildContext& ctx,
                         Tile t,
                         int side,
                         float seed) {
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
        if (!renderAssets_.airVentPropMesh.vertices.empty()) {
            float spanX = std::max(0.001f, PropMeshSpan(renderAssets_.airVentPropMesh, 0));
            float spanY = std::max(0.001f, PropMeshSpan(renderAssets_.airVentPropMesh, 1));
            float scale = std::min(ventW / spanX, ventH / spanY);
            scale = std::clamp(scale, 0.05f, 8.0f);
            if (AddInstancedStaticProp(renderAssets_.airVentPropMesh, center, yaw, scale, scale, scale,
                    instancedVertices, instancedIndices, pendingStaticInstances, instancedMeshRanges)) {
                XMFLOAT3 right{std::cos(yaw), 0.0f, -std::sin(yaw)};
                XMFLOAT3 up{0.0f, 1.0f, 0.0f};
                XMFLOAT3 forward{std::sin(yaw), 0.0f, std::cos(yaw)};
                XMFLOAT3 emitterPos = Add3(center, OrientedOffset(right, up, forward, 0.0f, lowVent ? 0.02f : -0.02f, 0.090f));
                effectRuntime_.steamEmitters.push_back({emitterPos, forward, PropPlacementTileHash(t.x, t.y, 45.0f + seed) * 5.0f, false});
                cameraRuntime_.propLookPoints.push_back(center);
                return;
            }
        }
        XMFLOAT3 right{std::cos(yaw), 0.0f, -std::sin(yaw)};
        XMFLOAT3 up{0.0f, 1.0f, 0.0f};
        XMFLOAT3 forward{std::sin(yaw), 0.0f, std::cos(yaw)};
        auto voff = [&](float x, float y, float z) {
            return Add3(center, OrientedOffset(right, up, forward, x, y, z));
        };
        AddOrientedBox(vertices, indices, voff(0.0f, 0.0f, 0.010f), {ventW * 0.50f, ventH * 0.50f, 0.010f}, yaw, 8.0f);
        AddOrientedBox(vertices, indices, voff(0.0f, 0.0f, 0.022f), {ventW * 0.38f, ventH * 0.31f, 0.006f}, yaw, 5.0f);
        AddOrientedBox(vertices, indices, voff(-ventW * 0.47f, 0.0f, 0.027f), {0.014f, ventH * 0.43f, 0.010f}, yaw, 8.0f);
        AddOrientedBox(vertices, indices, voff(ventW * 0.47f, 0.0f, 0.027f), {0.014f, ventH * 0.43f, 0.010f}, yaw, 8.0f);
        AddOrientedBox(vertices, indices, voff(0.0f, -ventH * 0.43f, 0.027f), {ventW * 0.45f, 0.012f, 0.010f}, yaw, 8.0f);
        AddOrientedBox(vertices, indices, voff(0.0f, ventH * 0.43f, 0.027f), {ventW * 0.45f, 0.012f, 0.010f}, yaw, 8.0f);
        for (int s = -3; s <= 3; ++s) {
            float yOff = static_cast<float>(s) * ventH * 0.115f;
            float stagger = (s & 1) ? 0.006f : -0.004f;
            AddOrientedBox(vertices, indices, voff(stagger, yOff, 0.034f + static_cast<float>(s + 3) * 0.0008f),
                {ventW * 0.36f, 0.006f, 0.007f}, yaw, 8.0f);
        }
        const XMFLOAT2 screws[] = {{-0.43f, -0.37f}, {0.43f, -0.37f}, {-0.43f, 0.37f}, {0.43f, 0.37f}};
        for (const XMFLOAT2& screw : screws) {
            AddOrientedBox(vertices, indices, voff(screw.x * ventW, screw.y * ventH, 0.039f), {0.014f, 0.014f, 0.005f}, yaw, 10.0f);
        }
        effectRuntime_.steamEmitters.push_back({voff(0.0f, lowVent ? 0.02f : -0.02f, 0.082f), forward,
            PropPlacementTileHash(t.x, t.y, 45.0f + seed) * 5.0f, false});
        cameraRuntime_.propLookPoints.push_back(center);
    }
