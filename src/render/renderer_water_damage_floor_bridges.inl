        for (int y = 1; y < RenderMazeView().h - 1; ++y) {
            for (int x = 1; x < RenderMazeView().w - 1; ++x) {
                Tile t{x, y};
                EmitFloorWaterBridge(build, t, 1);
                EmitFloorWaterBridge(build, t, 3);
            }
        }
