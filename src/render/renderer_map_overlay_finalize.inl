        markTile(cameraTile, {0.20f, 0.72f, 1.0f, 0.82f}, 1.70f);

        mapOverlayRuntime_.cachedVerts = verts;
        mapOverlayRuntime_.cacheWidth = hostRuntime_.width;
        mapOverlayRuntime_.cacheHeight = hostRuntime_.height;
        mapOverlayRuntime_.cacheMazeW = maze.w;
        mapOverlayRuntime_.cacheMazeH = maze.h;
        mapOverlayRuntime_.cacheStyle = sessionRuntime_.mapOverlayStyle;
        mapOverlayRuntime_.nextUpdateTime = timeRuntime_.time + (settingsRuntime_.live.debugAiMapOverlay ? 0.085f : 0.16f);
        DrawOverlayVertices(verts);
