        float mainHalfFov = fovDegrees * 0.5f * kPi / 180.0f;
        float mainHorizontalHalfFov = std::atan(std::tan(mainHalfFov) * std::max(0.1f, aspect));
        float mainConeCos = std::cos(std::min(kPi * 0.98f, std::max(mainHalfFov, mainHorizontalHalfFov) + 0.82f));
        float mainCullDistance = monsterPreview_.active
            ? 1000.0f
            : std::max(viewFarMeters, settingsRuntime_.live.fogEndMeters + (visibilityMaze ? visibilityMaze->TileAverage() : 0.1f) * 16.0f);
        float mainForceVisibleDistance = std::max((visibilityMaze ? visibilityMaze->TileAverage() : 0.1f) * 7.0f, 12.0f);
        float transparentCullDistance = monsterPreview_.active
            ? mainCullDistance
            : std::min(mainCullDistance, std::max((visibilityMaze ? visibilityMaze->TileAverage() : 0.1f) * 8.0f, settingsRuntime_.live.fogEndMeters + (visibilityMaze ? visibilityMaze->TileAverage() : 0.1f) * 4.0f));
        if (!monsterPreview_.active) {
            buildMazeVisibility(eyePos, viewDirFloat, mainCullDistance, mainConeCos, 6);
        }
