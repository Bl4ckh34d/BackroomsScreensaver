
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
