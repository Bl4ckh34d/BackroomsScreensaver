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
