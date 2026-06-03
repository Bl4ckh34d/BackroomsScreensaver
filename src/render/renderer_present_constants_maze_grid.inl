        cb.maze0 = {
            -static_cast<float>(lightingMaze.w) * lightingMaze.tileW * 0.5f,
            -static_cast<float>(lightingMaze.h) * lightingMaze.tileD * 0.5f,
            lightingMaze.tileW,
            lightingMaze.tileD
        };
        cb.maze1 = {
            static_cast<float>(lightingMaze.w),
            static_cast<float>(lightingMaze.h),
            settingsRuntime_.live.wallHeightMeters,
            tileAverage
        };
