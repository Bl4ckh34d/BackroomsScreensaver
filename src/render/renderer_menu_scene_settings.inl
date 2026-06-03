    void ApplyMainMenuSettings() {
        settingsRuntime_.live.mazeWidth = 3;
        settingsRuntime_.live.mazeHeight = 3;
        settingsRuntime_.live.wallHeightMeters = std::max(settingsRuntime_.live.wallHeightMeters, 2.85f);
        settingsRuntime_.live.mapOverlay = false;
        settingsRuntime_.live.debugAiMapOverlay = false;
        settingsRuntime_.live.chairDensity = 0.0f;
        settingsRuntime_.live.paperDensity = 0.0f;
        settingsRuntime_.live.hallwayPaperRunDensity = 0.0f;
        settingsRuntime_.live.metalCabinetDensity = 0.0f;
        settingsRuntime_.live.waterDamageDensity = 0.0f;
        settingsRuntime_.live.lampOnRatio = 1.0f;
        settingsRuntime_.live.lampSpacing = std::max(settingsRuntime_.live.tileWidthMeters, settingsRuntime_.live.tileLengthMeters) * 2.4f;
        settingsRuntime_.live.lampIntensity = std::max(settingsRuntime_.live.lampIntensity, 1.85f);
        settingsRuntime_.live.lampFlickerRatio = 0.080f;
        settingsRuntime_.live.brokenZoneRatio = 0.0f;
        settingsRuntime_.live.ambientLight = std::max(settingsRuntime_.live.ambientLight, 0.045f);
        settingsRuntime_.live.exposure = std::max(settingsRuntime_.live.exposure, 1.12f);
        settingsRuntime_.live.flashlightIntensity = std::clamp(settingsRuntime_.live.flashlightIntensity, 1.15f, 1.55f);
        settingsRuntime_.live.flashlightAttenuation = std::min(settingsRuntime_.live.flashlightAttenuation, 0.048f);
        settingsRuntime_.live.flashlightConeDegrees = std::clamp(settingsRuntime_.live.flashlightConeDegrees, 68.0f, 78.0f);
        settingsRuntime_.live.airParticles = true;
        settingsRuntime_.live.airParticleDensity = std::max(0.32f, settingsRuntime_.live.airParticleDensity * 0.55f);
        settingsRuntime_.live.sparkParticles = true;
        settingsRuntime_.live.fadeInSeconds = std::max(settingsRuntime_.live.fadeInSeconds, 1.85f);
        settingsRuntime_.live.bloodWorldCoverage = std::max(settingsRuntime_.live.bloodWorldCoverage, 0.45f);
        settingsRuntime_.live.bloodWorldAlwaysOn = false;
        settingsRuntime_.live.bloodWorldFlickerIntensity = 0.0f;
    }

    MenuPlaquePlacement MenuButtonPlacement(int index) const {
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        const Maze& maze = *world.maze;
        XMFLOAT3 c = maze.WorldCenter(maze.start, 0.0f);
        const float northWallZ = c.z + maze.tileD * 0.5f - 0.034f;
        MenuPlaquePlacement plaque{};
        plaque.halfW = std::min(maze.tileW * 0.68f, 1.10f);
        int count = std::clamp(menuRuntime_.buttonCount, 3, static_cast<int>(menuRuntime_.buttonLabelRows.size()));
        float spacing = count >= 6 ? 0.300f : (count >= 5 ? 0.325f : 0.360f);
        float topY = count >= 6 ? 1.90f : (count >= 5 ? 1.82f : 1.72f);
        plaque.halfH = count >= 5 ? 0.132f : 0.150f;
        plaque.center = {c.x + maze.tileW * 0.50f, topY - static_cast<float>(index) * spacing, northWallZ};
        plaque.right = {1.0f, 0.0f, 0.0f};
        plaque.inward = {0.0f, 0.0f, -1.0f};
        return plaque;
    }

    MenuPlaquePlacement MenuCustomPanelPlacement() const {
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        const Maze& maze = *world.maze;
        XMFLOAT3 c = maze.WorldCenter(maze.start, 0.0f);
        float eastWallX = c.x + maze.tileW * 2.5f - 0.034f;
        MenuPlaquePlacement plaque{};
        plaque.halfW = std::min(maze.tileD * 0.78f, 1.25f);
        plaque.halfH = std::min(maze.tileD * 0.48f, 0.76f);
        plaque.center = {eastWallX, 1.54f, c.z - maze.tileD * 0.80f};
        plaque.right = {0.0f, 0.0f, -1.0f};
        plaque.inward = {-1.0f, 0.0f, 0.0f};
        return plaque;
    }
