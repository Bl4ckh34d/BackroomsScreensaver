        for (int side = 2; side < 4; ++side) {
            for (int x = 0; x < RenderMazeView().w; ++x) {
                int y = 0;
                while (y < RenderMazeView().h) {
                    while (y < RenderMazeView().h && !surfaceAt(x, y, side)) ++y;
                    int start = y;
                    while (y < RenderMazeView().h && surfaceAt(x, y, side)) ++y;
                    if (start < y) emitRun(side, x, start, y - 1);
                }
            }
        }
