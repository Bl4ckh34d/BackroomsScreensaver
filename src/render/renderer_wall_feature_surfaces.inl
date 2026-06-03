    void AddWallFeatureInteriorSurfaces(std::vector<Vertex>& vertices,
                                        std::vector<uint32_t>& indices,
                                        const MazeSurfaceBuildContext& ctx) {
        for (int y = 1; y < RenderMazeView().h - 1; ++y) {
            for (int x = 1; x < RenderMazeView().w - 1; ++x) {
                MazeWallFeature feature = RenderMazeView().WallFeature(x, y);
                if (feature == MazeWallFeature::None || RenderMazeView().IsOpen(x, y)) continue;

                const bool east = RenderMazeView().IsOpen(x + 1, y);
                const bool west = RenderMazeView().IsOpen(x - 1, y);
                const bool south = RenderMazeView().IsOpen(x, y + 1);
                const bool north = RenderMazeView().IsOpen(x, y - 1);
                const bool eastWestPassage = east && west && !south && !north;
                const bool northSouthPassage = south && north && !east && !west;
                if (!eastWestPassage && !northSouthPassage) continue;

                auto adjacentFeature = [&](int tx, int ty) {
                    return RenderMazeView().WallFeature(tx, ty) != MazeWallFeature::None && !RenderMazeView().IsOpen(tx, ty);
                };

                const float l = ctx.ox + x * ctx.tileW;
                const float r = l + ctx.tileW;
                const float z0 = ctx.oz + y * ctx.tileD;
                const float z1 = z0 + ctx.tileD;
                const float splitY = feature == MazeWallFeature::Window ? ctx.wallFeatureWindowSplitY : ctx.wallFeatureTunnelSplitY;
                const float capY = std::clamp(splitY, 0.02f, ctx.wallH - 0.02f);

                if (eastWestPassage) {
                    if (!adjacentFeature(x, y - 1)) {
                        AddQuadUV(vertices, indices,
                            {l, 0.0f, z0}, {r, 0.0f, z0}, {r, ctx.wallH, z0}, {l, ctx.wallH, z0},
                            {0, 0, 1}, {1, 0, 0},
                            WallUvX(l, 0.0f), WallUvX(r, 0.0f), WallUvX(r, ctx.wallH), WallUvX(l, ctx.wallH), 0.0f);
                    }
                    if (!adjacentFeature(x, y + 1)) {
                        AddQuadUV(vertices, indices,
                            {r, 0.0f, z1}, {l, 0.0f, z1}, {l, ctx.wallH, z1}, {r, ctx.wallH, z1},
                            {0, 0, -1}, {-1, 0, 0},
                            WallUvX(r, 0.0f), WallUvX(l, 0.0f), WallUvX(l, ctx.wallH), WallUvX(r, ctx.wallH), 0.0f);
                    }
                } else {
                    if (!adjacentFeature(x - 1, y)) {
                        AddQuadUV(vertices, indices,
                            {l, 0.0f, z1}, {l, 0.0f, z0}, {l, ctx.wallH, z0}, {l, ctx.wallH, z1},
                            {1, 0, 0}, {0, 0, -1},
                            WallUvZ(z1, 0.0f), WallUvZ(z0, 0.0f), WallUvZ(z0, ctx.wallH), WallUvZ(z1, ctx.wallH), 0.0f);
                    }
                    if (!adjacentFeature(x + 1, y)) {
                        AddQuadUV(vertices, indices,
                            {r, 0.0f, z0}, {r, 0.0f, z1}, {r, ctx.wallH, z1}, {r, ctx.wallH, z0},
                            {-1, 0, 0}, {0, 0, 1},
                            WallUvZ(z0, 0.0f), WallUvZ(z1, 0.0f), WallUvZ(z1, ctx.wallH), WallUvZ(z0, ctx.wallH), 0.0f);
                    }
                }

                if (feature == MazeWallFeature::Window) {
                    AddQuadUV(vertices, indices,
                        {l, capY, z1}, {r, capY, z1}, {r, capY, z0}, {l, capY, z0},
                        {0, 1, 0}, {1, 0, 0},
                        FloorUv(l, z1), FloorUv(r, z1), FloorUv(r, z0), FloorUv(l, z0), 0.0f);
                } else {
                    AddQuadUV(vertices, indices,
                        {l, capY, z0}, {r, capY, z0}, {r, capY, z1}, {l, capY, z1},
                        {0, -1, 0}, {1, 0, 0},
                        CeilingUv(l, z0), CeilingUv(r, z0), CeilingUv(r, z1), CeilingUv(l, z1), 0.0f);
                }
            }
        }
    }
