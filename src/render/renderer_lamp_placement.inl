// Ceiling lamp placement helpers.
// Included inside Renderer private section before maze mesh construction.

    void AddMazeCeilingLamps(std::vector<Vertex>& vertices,
                             std::vector<uint32_t>& indices,
                             const MazeSurfaceBuildContext& ctx) {
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        if (!world.maze) return;
        const Maze& maze = *world.maze;
        auto mainMenuLampAllowed = [&](Tile lampTile) {
            return MainMenuAllowedLampTile(lampTile);
        };

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
                float panelW = ctx.tileW * (0.94f / 3.0f);
                float panelD = ctx.tileD * (0.94f / 3.0f);
                float material = lampOn ? 3.0f + seed * 0.49f : 5.0f;
                AddCeilingCard(vertices, indices, {lampCenter.x, 0.0f, lampCenter.z},
                    panelW, panelD, 0.0f, ctx.wallH - 0.004f, material);

                if (lampOn) {
                    effectRuntime_.runtimeLamps.push_back({
                        lampTile,
                        {lampCenter.x, ctx.wallH - 0.08f, lampCenter.z},
                        0.0f,
                        RandRange(0.08f, 0.72f),
                        false,
                        [&]() {
                            float humRoll = LampHash(static_cast<float>(cellX) + 14.7f, static_cast<float>(cellZ) - 42.3f);
                            if (humRoll < 0.05f) return 2;
                            if (humRoll < 0.15f) return 1;
                            return 0;
                        }()
                    });
                    if (jumpscareLamp && settingsRuntime_.live.sparkParticles) {
                        effectRuntime_.sparkEmitters.push_back({{lampCenter.x, ctx.wallH - 0.085f, lampCenter.z}});
                    }
                } else if (((sessionRuntime_.mode == RendererRuntimeMode::MainMenu && brokenPanel) || wetLampTile) &&
                           brokenPanel && settingsRuntime_.live.sparkParticles) {
                    SparkEmitter emitter{{lampCenter.x, ctx.wallH - 0.085f, lampCenter.z}};
                    if (sessionRuntime_.mode == RendererRuntimeMode::MainMenu) {
                        emitter.cooldown = 1.4f + LampHash(static_cast<float>(cellX) + 51.2f, static_cast<float>(cellZ) - 17.4f) * 4.8f;
                    }
                    effectRuntime_.sparkEmitters.push_back(emitter);
                }
                if (brokenZone && !effectRuntime_.lampDamagePixels.empty()) {
                    effectRuntime_.lampDamagePixels[static_cast<size_t>(tileY * maze.w + tileX)] = 255;
                    effectRuntime_.lampDamageDirty = true;
                }
                if (forceLampOff && !effectRuntime_.lampDamagePixels.empty()) {
                    effectRuntime_.lampDamagePixels[static_cast<size_t>(tileY * maze.w + tileX)] = 255;
                    effectRuntime_.lampDamageDirty = true;
                }
            }
        }
        UpdateMainMenuLampOverrides();
        CreateLampDamageTexture();
    }
