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
