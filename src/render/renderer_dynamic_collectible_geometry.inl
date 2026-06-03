    void AppendCollectiblePages(std::vector<Vertex>& verts) {
        if (!IsPlayableSimulationMode(sessionRuntime_.mode)) return;
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        if (!world.maze || !world.collectiblePages) return;
        constexpr float halfW = 0.210f * 0.5f;
        constexpr float halfH = 0.297f * 0.5f;
        for (const CollectiblePage& page : *world.collectiblePages) {
            if (page.collected || page.pageIndex < 0 || page.pageIndex >= kCollectiblePageMaterialCount) continue;
            if (!DynamicVisualCandidate(page.center, 0.24f, std::max(8.0f, world.maze->TileAverage() * 7.0f))) continue;
            XMFLOAT3 right = Normalize3(page.right, {1.0f, 0.0f, 0.0f});
            XMFLOAT3 up = Normalize3(page.up, {0.0f, 1.0f, 0.0f});
            XMFLOAT3 normal = Normalize3(page.normal, Normalize3(Cross3(right, up), {0.0f, 1.0f, 0.0f}));
            XMFLOAT3 hw = Scale3(right, halfW);
            XMFLOAT3 hh = Scale3(up, halfH);
            XMFLOAT3 a = Add3(page.center, Add3(Scale3(hw, -1.0f), Scale3(hh, -1.0f)));
            XMFLOAT3 b = Add3(page.center, Add3(hw, Scale3(hh, -1.0f)));
            XMFLOAT3 c = Add3(page.center, Add3(hw, hh));
            XMFLOAT3 d = Add3(page.center, Add3(Scale3(hw, -1.0f), hh));
            AppendDynamicQuadUV(verts, a, b, c, d, normal, right,
                {1.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 0.0f},
                static_cast<float>(kCollectiblePageMaterialFirst + page.pageIndex));
        }
    }

    void AppendSavePoint(std::vector<Vertex>& verts) {
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        if (!world.maze) return;
        const SavePoint& savePoint = world.savePoint;
        if (sessionRuntime_.mode != RendererRuntimeMode::PlayableGame || !savePoint.active) return;
        if (!DynamicVisualCandidate(Add3(savePoint.pos, {0.0f, 0.72f, 0.0f}), 0.85f, std::max(9.0f, world.maze->TileAverage() * 7.0f))) return;
        XMFLOAT3 right{std::cos(savePoint.yaw), 0.0f, -std::sin(savePoint.yaw)};
        XMFLOAT3 up{0.0f, 1.0f, 0.0f};
        XMFLOAT3 forward{std::sin(savePoint.yaw), 0.0f, std::cos(savePoint.yaw)};
        XMFLOAT3 base = savePoint.pos;
        AppendDynamicBoxAxes(verts, Add3(base, {0.0f, 0.22f, 0.0f}), right, up, forward, {0.30f, 0.22f, 0.22f}, 19.0f);
        AppendDynamicBoxAxes(verts, Add3(base, {0.0f, 0.49f, 0.0f}), right, up, forward, {0.44f, 0.055f, 0.30f}, 22.0f);
        AppendDynamicBoxAxes(verts, Add3(base, {0.0f, 0.62f, 0.02f}), right, up, forward, {0.28f, 0.075f, 0.17f}, 23.0f);
        AppendDynamicBoxAxes(verts, Add3(base, Add3(Scale3(forward, -0.13f), {0.0f, 0.73f, 0.0f})), right, up, forward, {0.24f, 0.070f, 0.035f}, 18.0f);
        AppendDynamicBoxAxes(verts, Add3(base, Add3(Scale3(forward, 0.17f), {0.0f, 0.68f, 0.0f})), right, up, forward, {0.20f, 0.030f, 0.045f}, 18.0f);
        for (int i = -4; i <= 4; ++i) {
            float x = static_cast<float>(i) * 0.045f;
            AppendDynamicBoxAxes(verts, Add3(base, Add3(Scale3(right, x), Add3(Scale3(forward, 0.12f), {0.0f, 0.72f, 0.0f}))),
                right, up, forward, {0.014f, 0.014f, 0.018f}, 21.0f);
        }
    }
