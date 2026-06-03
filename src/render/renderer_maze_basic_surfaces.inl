// Maze wall, floor, and ceiling surface helpers.
// Included inside Renderer private section before placement helpers.

    struct MazeSurfaceBuildContext {
        float ox = 0.0f;
        float oz = 0.0f;
        float tileW = kTile;
        float tileD = kTile;
        float wallH = 3.0f;
        float wallFeatureWindowSplitY = 1.2f;
        float wallFeatureTunnelSplitY = 1.6f;
    };

    void AddMazeFloorTile(std::vector<Vertex>& vertices,
                          std::vector<uint32_t>& indices,
                          const MazeSurfaceBuildContext& ctx,
                          int x,
                          int y) {
        float z0 = ctx.oz + y * ctx.tileD;
        float z1 = z0 + ctx.tileD;
        float l = ctx.ox + x * ctx.tileW;
        float r = l + ctx.tileW;
        AddQuadUV(vertices, indices,
            {l, 0, z1}, {r, 0, z1}, {r, 0, z0}, {l, 0, z0},
            {0, 1, 0}, {1, 0, 0},
            FloorUv(l, z1), FloorUv(r, z1), FloorUv(r, z0), FloorUv(l, z0), 1.0f);
    }

    void AddMazeCeilingTile(std::vector<Vertex>& vertices,
                            std::vector<uint32_t>& indices,
                            const MazeSurfaceBuildContext& ctx,
                            int x,
                            int y) {
        float z0 = ctx.oz + y * ctx.tileD;
        float z1 = z0 + ctx.tileD;
        float l = ctx.ox + x * ctx.tileW;
        float r = l + ctx.tileW;
        AddQuadUV(vertices, indices,
            {l, ctx.wallH, z0}, {r, ctx.wallH, z0}, {r, ctx.wallH, z1}, {l, ctx.wallH, z1},
            {0, -1, 0}, {1, 0, 0},
            CeilingUv(l, z0), CeilingUv(r, z0), CeilingUv(r, z1), CeilingUv(l, z1), 2.0f);
    }

    void AddMazeFloorCeilingRun(std::vector<Vertex>& vertices,
                                std::vector<uint32_t>& indices,
                                const MazeSurfaceBuildContext& ctx,
                                int y,
                                int x0,
                                int x1) {
        for (int x = x0; x < x1; ++x) {
            AddMazeFloorTile(vertices, indices, ctx, x, y);
            AddMazeCeilingTile(vertices, indices, ctx, x, y);
        }
    }

    void AddNorthWallRun(std::vector<Vertex>& vertices,
                         std::vector<uint32_t>& indices,
                         const MazeSurfaceBuildContext& ctx,
                         int y,
                         int x0,
                         int x1) {
        float z = ctx.oz + y * ctx.tileD;
        for (int x = x0; x < x1; ++x) {
            float l = ctx.ox + x * ctx.tileW;
            float r = l + ctx.tileW;
            float y0 = 0.0f;
            float y1 = ctx.wallH;
            MazeWallFeature feature = RenderMazeView().WallFeature(x, y - 1);
            if (feature == MazeWallFeature::Window) y1 = ctx.wallFeatureWindowSplitY;
            if (feature == MazeWallFeature::Tunnel) y0 = ctx.wallFeatureTunnelSplitY;
            if (y1 <= y0 + 0.02f) continue;
            AddQuadUV(vertices, indices,
                {r, y0, z}, {l, y0, z}, {l, y1, z}, {r, y1, z},
                {0, 0, 1}, {-1, 0, 0},
                WallUvX(r, y0), WallUvX(l, y0), WallUvX(l, y1), WallUvX(r, y1), 0.0f);
        }
    }

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

    void AddEastWallRun(std::vector<Vertex>& vertices,
                        std::vector<uint32_t>& indices,
                        const MazeSurfaceBuildContext& ctx,
                        int x,
                        int y0,
                        int y1) {
        float r = ctx.ox + (x + 1) * ctx.tileW;
        for (int y = y0; y < y1; ++y) {
            float z0 = ctx.oz + y * ctx.tileD;
            float z1 = z0 + ctx.tileD;
            float wallY0 = 0.0f;
            float wallY1 = ctx.wallH;
            MazeWallFeature feature = RenderMazeView().WallFeature(x + 1, y);
            if (feature == MazeWallFeature::Window) wallY1 = ctx.wallFeatureWindowSplitY;
            if (feature == MazeWallFeature::Tunnel) wallY0 = ctx.wallFeatureTunnelSplitY;
            if (wallY1 <= wallY0 + 0.02f) continue;
            AddQuadUV(vertices, indices,
                {r, wallY0, z1}, {r, wallY0, z0}, {r, wallY1, z0}, {r, wallY1, z1},
                {-1, 0, 0}, {0, 0, -1},
                WallUvZ(z1, wallY0), WallUvZ(z0, wallY0), WallUvZ(z0, wallY1), WallUvZ(z1, wallY1), 0.0f);
        }
    }
