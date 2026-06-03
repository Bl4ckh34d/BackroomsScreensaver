    void AddSouthWallRun(std::vector<Vertex>& vertices,
                         std::vector<uint32_t>& indices,
                         const MazeSurfaceBuildContext& ctx,
                         int y,
                         int x0,
                         int x1) {
        float z = ctx.oz + (y + 1) * ctx.tileD;
        for (int x = x0; x < x1; ++x) {
            float l = ctx.ox + x * ctx.tileW;
            float r = l + ctx.tileW;
            float y0 = 0.0f;
            float y1 = ctx.wallH;
            MazeWallFeature feature = RenderMazeView().WallFeature(x, y + 1);
            if (feature == MazeWallFeature::Window) y1 = ctx.wallFeatureWindowSplitY;
            if (feature == MazeWallFeature::Tunnel) y0 = ctx.wallFeatureTunnelSplitY;
            if (y1 <= y0 + 0.02f) continue;
            AddQuadUV(vertices, indices,
                {l, y0, z}, {r, y0, z}, {r, y1, z}, {l, y1, z},
                {0, 0, -1}, {1, 0, 0},
                WallUvX(l, y0), WallUvX(r, y0), WallUvX(r, y1), WallUvX(l, y1), 0.0f);
        }
    }
