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
        if (!DynamicVisualCandidate(Add3(savePoint.pos, {0.0f, 0.53f, 0.0f}), 0.75f, std::max(9.0f, world.maze->TileAverage() * 7.0f))) return;
        static const std::vector<Vertex> mesh = [this] {
            std::vector<Vertex> model;
            model.reserve(9000);
            BuildSavePointTypewriter(model);
            return model;
        }();
        if (verts.size() + mesh.size() > kDynamicVertexCapacity) return;
        const float cosine = std::cos(savePoint.yaw);
        const float sine = std::sin(savePoint.yaw);
        auto rotate = [&](XMFLOAT3 v) {
            return XMFLOAT3{cosine * v.x + sine * v.z, v.y, -sine * v.x + cosine * v.z};
        };
        for (const Vertex& vertex : mesh) {
            verts.push_back({Add3(savePoint.pos, rotate(vertex.pos)),
                rotate(vertex.normal), rotate(vertex.tangent), vertex.uv, vertex.material});
        }
    }
