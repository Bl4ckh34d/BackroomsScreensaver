    void AddEastWallRun(std::vector<Vertex>& vertices,
                        std::vector<uint32_t>& indices,
                        const MazeSurfaceBuildContext& ctx,
                        int x,
                        int y0,
                        int y1) {
        float r = ctx.ox + (x + 1) * ctx.tileW;
        int runStart = y0;
        float runWallY0 = 0.0f;
        float runWallY1 = -1.0f;
        auto flushRun = [&](int runEnd) {
            if (runEnd <= runStart || runWallY1 <= runWallY0 + 0.02f) return;
            float z0 = ctx.oz + runStart * ctx.tileD;
            float z1 = ctx.oz + runEnd * ctx.tileD;
            AddQuadUV(vertices, indices,
                {r, runWallY0, z1}, {r, runWallY0, z0}, {r, runWallY1, z0}, {r, runWallY1, z1},
                {-1, 0, 0}, {0, 0, -1},
                WallUvZ(z1, runWallY0), WallUvZ(z0, runWallY0), WallUvZ(z0, runWallY1), WallUvZ(z1, runWallY1), 0.0f);
        };
        for (int y = y0; y < y1; ++y) {
            float wallY0 = 0.0f;
            float wallY1 = ctx.wallH;
            MazeWallFeature feature = RenderMazeView().WallFeature(x + 1, y);
            if (feature == MazeWallFeature::Window) wallY1 = ctx.wallFeatureWindowSplitY;
            if (feature == MazeWallFeature::Tunnel) wallY0 = ctx.wallFeatureTunnelSplitY;
            if (wallY1 <= wallY0 + 0.02f) {
                flushRun(y);
                runWallY1 = -1.0f;
                continue;
            }
            if (runWallY1 < 0.0f) {
                runStart = y;
                runWallY0 = wallY0;
                runWallY1 = wallY1;
            } else if (wallY0 != runWallY0 || wallY1 != runWallY1) {
                flushRun(y);
                runStart = y;
                runWallY0 = wallY0;
                runWallY1 = wallY1;
            }
        }
        flushRun(y1);
    }
