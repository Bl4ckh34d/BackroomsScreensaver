        for (int side = 0; side < 2; ++side) {
            for (int y = 0; y < RenderMazeView().h; ++y) {
                int x = 0;
                while (x < RenderMazeView().w) {
                    while (x < RenderMazeView().w && !surfaceAt(x, y, side)) ++x;
                    int start = x;
                    while (x < RenderMazeView().w && surfaceAt(x, y, side)) ++x;
                    if (start < x) emitRun(side, y, start, x - 1);
                }
            }
        }
