        const float tileW = maze.tileW;
        const float tileD = maze.tileD;
        const float tileAvg = maze.TileAverage();
        const float tileMin = maze.TileMinimum();
        const float wallH = settingsRuntime_.live.wallHeightMeters;
        const float wallFeatureWindowSplitY = std::clamp(std::max(0.86f, wallH * 0.40f), 0.68f, wallH - 0.08f);
        const float wallFeatureTunnelSplitY = std::clamp(std::max(1.18f, wallH * 0.54f), 0.86f, wallH - 0.08f);
        float ox = -static_cast<float>(maze.w) * tileW * 0.5f;
        float oz = -static_cast<float>(maze.h) * tileD * 0.5f;

        ExitPortal exitPortal = BuildExitPortal(tileW, tileD);
        const MazeSurfaceBuildContext surfaceBuild{
            ox,
            oz,
            tileW,
            tileD,
            wallH,
            wallFeatureWindowSplitY,
            wallFeatureTunnelSplitY
        };

        AddMazeWallRunsWithExitPortal(vertices, indices, surfaceBuild, exitPortal, tileAvg);
