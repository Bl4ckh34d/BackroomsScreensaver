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
