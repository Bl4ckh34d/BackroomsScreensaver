    void AddMazeWallRunsWithExitPortal(std::vector<Vertex>& vertices,
                                       std::vector<uint32_t>& indices,
                                       const MazeSurfaceBuildContext& ctx,
                                       const ExitPortal& exitPortal,
                                       float tileAvg) {
        auto addNorthWallRunWithPortal = [&](int y, int x0, int x1) {
            if (exitPortal.valid && exitPortal.dy == -1 && exitPortal.tile.y == y &&
                exitPortal.tile.x >= x0 && exitPortal.tile.x < x1) {
                if (x0 < exitPortal.tile.x) AddNorthWallRun(vertices, indices, ctx, y, x0, exitPortal.tile.x);
                AddExitDoorwayWallRun(vertices, indices, ctx, exitPortal, tileAvg);
                if (exitPortal.tile.x + 1 < x1) AddNorthWallRun(vertices, indices, ctx, y, exitPortal.tile.x + 1, x1);
            } else {
                AddNorthWallRun(vertices, indices, ctx, y, x0, x1);
            }
        };

        auto addSouthWallRunWithPortal = [&](int y, int x0, int x1) {
            if (exitPortal.valid && exitPortal.dy == 1 && exitPortal.tile.y == y &&
                exitPortal.tile.x >= x0 && exitPortal.tile.x < x1) {
                if (x0 < exitPortal.tile.x) AddSouthWallRun(vertices, indices, ctx, y, x0, exitPortal.tile.x);
                AddExitDoorwayWallRun(vertices, indices, ctx, exitPortal, tileAvg);
                if (exitPortal.tile.x + 1 < x1) AddSouthWallRun(vertices, indices, ctx, y, exitPortal.tile.x + 1, x1);
            } else {
                AddSouthWallRun(vertices, indices, ctx, y, x0, x1);
            }
        };

        auto addWestWallRunWithPortal = [&](int x, int y0, int y1) {
            if (exitPortal.valid && exitPortal.dx == -1 && exitPortal.tile.x == x &&
                exitPortal.tile.y >= y0 && exitPortal.tile.y < y1) {
                if (y0 < exitPortal.tile.y) AddWestWallRun(vertices, indices, ctx, x, y0, exitPortal.tile.y);
                AddExitDoorwayWallRun(vertices, indices, ctx, exitPortal, tileAvg);
                if (exitPortal.tile.y + 1 < y1) AddWestWallRun(vertices, indices, ctx, x, exitPortal.tile.y + 1, y1);
            } else {
                AddWestWallRun(vertices, indices, ctx, x, y0, y1);
            }
        };

        auto addEastWallRunWithPortal = [&](int x, int y0, int y1) {
            if (exitPortal.valid && exitPortal.dx == 1 && exitPortal.tile.x == x &&
                exitPortal.tile.y >= y0 && exitPortal.tile.y < y1) {
                if (y0 < exitPortal.tile.y) AddEastWallRun(vertices, indices, ctx, x, y0, exitPortal.tile.y);
                AddExitDoorwayWallRun(vertices, indices, ctx, exitPortal, tileAvg);
                if (exitPortal.tile.y + 1 < y1) AddEastWallRun(vertices, indices, ctx, x, exitPortal.tile.y + 1, y1);
            } else {
                AddEastWallRun(vertices, indices, ctx, x, y0, y1);
            }
        };

        for (int y = 0; y < RenderMazeView().h; ++y) {
            int x = 0;
            while (x < RenderMazeView().w) {
                while (x < RenderMazeView().w && !(RenderMazeView().IsOpen(x, y) && !RenderMazeView().IsOpen(x, y - 1))) ++x;
                int start = x;
                while (x < RenderMazeView().w && RenderMazeView().IsOpen(x, y) && !RenderMazeView().IsOpen(x, y - 1)) ++x;
                if (start < x) addNorthWallRunWithPortal(y, start, x);
            }

            x = 0;
            while (x < RenderMazeView().w) {
                while (x < RenderMazeView().w && !(RenderMazeView().IsOpen(x, y) && !RenderMazeView().IsOpen(x, y + 1))) ++x;
                int start = x;
                while (x < RenderMazeView().w && RenderMazeView().IsOpen(x, y) && !RenderMazeView().IsOpen(x, y + 1)) ++x;
                if (start < x) addSouthWallRunWithPortal(y, start, x);
            }
        }

        for (int x = 0; x < RenderMazeView().w; ++x) {
            int y = 0;
            while (y < RenderMazeView().h) {
                while (y < RenderMazeView().h && !(RenderMazeView().IsOpen(x, y) && !RenderMazeView().IsOpen(x - 1, y))) ++y;
                int start = y;
                while (y < RenderMazeView().h && RenderMazeView().IsOpen(x, y) && !RenderMazeView().IsOpen(x - 1, y)) ++y;
                if (start < y) addWestWallRunWithPortal(x, start, y);
            }

            y = 0;
            while (y < RenderMazeView().h) {
                while (y < RenderMazeView().h && !(RenderMazeView().IsOpen(x, y) && !RenderMazeView().IsOpen(x + 1, y))) ++y;
                int start = y;
                while (y < RenderMazeView().h && RenderMazeView().IsOpen(x, y) && !RenderMazeView().IsOpen(x + 1, y)) ++y;
                if (start < y) addEastWallRunWithPortal(x, start, y);
            }
        }

        AddWallFeatureInteriorSurfaces(vertices, indices, ctx);
    }
