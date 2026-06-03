    void AddWestWallRun(std::vector<Vertex>& vertices,
                        std::vector<uint32_t>& indices,
                        const MazeSurfaceBuildContext& ctx,
                        int x,
                        int y0,
                        int y1) {
        float l = ctx.ox + x * ctx.tileW;
        for (int y = y0; y < y1; ++y) {
            float z0 = ctx.oz + y * ctx.tileD;
            float z1 = z0 + ctx.tileD;
            float wallY0 = 0.0f;
            float wallY1 = ctx.wallH;
            MazeWallFeature feature = RenderMazeView().WallFeature(x - 1, y);
            if (feature == MazeWallFeature::Window) wallY1 = ctx.wallFeatureWindowSplitY;
            if (feature == MazeWallFeature::Tunnel) wallY0 = ctx.wallFeatureTunnelSplitY;
            if (wallY1 <= wallY0 + 0.02f) continue;
            AddQuadUV(vertices, indices,
                {l, wallY0, z0}, {l, wallY0, z1}, {l, wallY1, z1}, {l, wallY1, z0},
                {1, 0, 0}, {0, 0, 1},
                WallUvZ(z0, wallY0), WallUvZ(z1, wallY0), WallUvZ(z1, wallY1), WallUvZ(z0, wallY1), 0.0f);
        }
    }
