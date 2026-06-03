        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        const Maze* mapMaze = world.maze;
        if ((!settingsRuntime_.live.mapOverlay && !settingsRuntime_.live.debugAiMapOverlay) || !renderBuffers_.overlayBuffer || !shaders_.overlayVertexShader || !shaders_.overlayPixelShader ||
            hostRuntime_.width <= 0 || hostRuntime_.height <= 0 || !mapMaze || mapMaze->w <= 0 || mapMaze->h <= 0 || mapMaze->open.empty()) {
            return;
        }
        const Maze& maze = *mapMaze;
        static const std::vector<Tile> kEmptyMonsterPath;
        static const std::vector<PlayerAudibleSoundPulse> kEmptySoundPulses;
        const std::vector<Tile>& monsterPath = world.monsterPath ? *world.monsterPath : kEmptyMonsterPath;
        const std::vector<PlayerAudibleSoundPulse>& soundPulses = world.playerSoundPulses ? *world.playerSoundPulses : kEmptySoundPulses;

        bool cacheValid = !mapOverlayRuntime_.cachedVerts.empty() &&
            mapOverlayRuntime_.cacheWidth == hostRuntime_.width &&
            mapOverlayRuntime_.cacheHeight == hostRuntime_.height &&
            mapOverlayRuntime_.cacheMazeW == maze.w &&
            mapOverlayRuntime_.cacheMazeH == maze.h &&
            mapOverlayRuntime_.cacheStyle == sessionRuntime_.mapOverlayStyle;
        if (cacheValid && timeRuntime_.time < mapOverlayRuntime_.nextUpdateTime) {
            DrawOverlayVertices(mapOverlayRuntime_.cachedVerts);
            return;
        }
