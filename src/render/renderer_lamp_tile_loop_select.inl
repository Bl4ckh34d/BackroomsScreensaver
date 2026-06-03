        for (int tileY = 0; tileY < maze.h; ++tileY) {
            for (int tileX = 0; tileX < maze.w; ++tileX) {
                MazeWallFeature feature = maze.WallFeature(tileX, tileY);
                bool hasCeilingSurface = maze.IsOpen(tileX, tileY) || feature == MazeWallFeature::Window;
                if (!hasCeilingSurface) continue;

                Tile lampTile{tileX, tileY};
                XMFLOAT3 lampCenter = maze.WorldCenter(lampTile, 0.0f);
                int cellX = tileX;
                int cellZ = tileY;
                float seed = LampSeed(cellX, cellZ);
                bool brokenZone = LampBrokenZone(cellX, cellZ);
                bool lampOn = !brokenZone && seed >= 1.0f - settingsRuntime_.live.lampOnRatio;
                bool brokenPanel = brokenZone &&
                    LampHash(static_cast<float>(cellX) - 19.7f, static_cast<float>(cellZ) + 88.4f) < settingsRuntime_.live.sparkEmitterRatio;
                bool wetLampTile = IsWetCeilingTile(lampTile) || IsWetFootstepTile(lampTile);
                bool jumpscareLamp = wetLampTile && lampOn && IsPlayableSimulationMode(sessionRuntime_.mode) &&
                    LampHash(static_cast<float>(cellX) + 151.3f, static_cast<float>(cellZ) - 207.9f) < settingsRuntime_.live.sparkEmitterRatio;
                bool forceLampOff = false;
                if (sessionRuntime_.mode == RendererRuntimeMode::MainMenu) {
                    Tile menuSparkTile{std::clamp(maze.start.x + 1, 1, maze.w - 2), std::max(1, maze.start.y - 5)};
                    bool menuAllowedLamp = mainMenuLampAllowed(lampTile);
                    bool menuBrokenPanel = lampTile == menuSparkTile && !MainMenuAlwaysLitLampTile(lampTile);
                    brokenZone = menuBrokenPanel;
                    forceLampOff = !menuAllowedLamp;
                    lampOn = menuAllowedLamp && !menuBrokenPanel;
                    brokenPanel = menuBrokenPanel;
                    jumpscareLamp = false;
                }
