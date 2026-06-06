    void AddSouthWallRun(std::vector<Vertex>& vertices,
                         std::vector<uint32_t>& indices,
                         const MazeSurfaceBuildContext& ctx,
                         int y,
                         int x0,
                         int x1) {
        float z = ctx.oz + (y + 1) * ctx.tileD;
        int runStart = x0;
        float runY0 = 0.0f;
        float runY1 = -1.0f;
        auto flushRun = [&](int runEnd) {
            if (runEnd <= runStart || runY1 <= runY0 + 0.02f) return;
            float l = ctx.ox + runStart * ctx.tileW;
            float r = ctx.ox + runEnd * ctx.tileW;
            AddQuadUV(vertices, indices,
                {l, runY0, z}, {r, runY0, z}, {r, runY1, z}, {l, runY1, z},
                {0, 0, -1}, {1, 0, 0},
                WallUvX(l, runY0), WallUvX(r, runY0), WallUvX(r, runY1), WallUvX(l, runY1), 0.0f);
        };
        for (int x = x0; x < x1; ++x) {
            float y0 = 0.0f;
            float y1 = ctx.wallH;
            MazeWallFeature feature = RenderMazeView().WallFeature(x, y + 1);
            if (feature == MazeWallFeature::Window) y1 = ctx.wallFeatureWindowSplitY;
            if (feature == MazeWallFeature::Tunnel) y0 = ctx.wallFeatureTunnelSplitY;
            if (y1 <= y0 + 0.02f) {
                flushRun(x);
                runY1 = -1.0f;
                continue;
            }
            if (runY1 < 0.0f) {
                runStart = x;
                runY0 = y0;
                runY1 = y1;
            } else if (y0 != runY0 || y1 != runY1) {
                flushRun(x);
                runStart = x;
                runY0 = y0;
                runY1 = y1;
            }
        }
        flushRun(x1);
    }
