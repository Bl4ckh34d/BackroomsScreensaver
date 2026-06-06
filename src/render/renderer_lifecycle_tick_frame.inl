    void TickFrame(float dt) {
        const bool runtimeProfile = RuntimeProfileEnabled();
        double profileStart = 0.0;
        double profileAfterProgress = 0.0;
        double profileAfterBudget = 0.0;
        double profileAfterSimulation = 0.0;
        double profileAfterAudio = 0.0;
        if (runtimeProfile) profileStart = ProfileNowMs();
        timeRuntime_.time += dt;
        if (AutoplayBenchmarkEnabled()) {
            benchmarkRuntime_.autoplayActive = true;
            benchmarkRuntime_.autoplayTimer += std::max(0.0f, dt);
            float duration = AutoplayBenchmarkDurationSeconds();
            if (duration > 0.0f &&
                benchmarkRuntime_.autoplayTimer >= duration &&
                !benchmarkRuntime_.autoplayClosePosted &&
                hostRuntime_.hwnd) {
                benchmarkRuntime_.autoplayClosePosted = true;
                PostMessageW(hostRuntime_.hwnd, WM_CLOSE, 0, 0);
            }
        }
        UpdatePlayableProgressionTimers(dt);
        if (runtimeProfile) profileAfterProgress = ProfileNowMs();
        UpdateAirParticlePerformanceBudget(dt);
        if (runtimeProfile) profileAfterBudget = ProfileNowMs();
        UpdateSimulation(dt);
        if (runtimeProfile) profileAfterSimulation = ProfileNowMs();
        UpdateAudio(dt);
        if (runtimeProfile) profileAfterAudio = ProfileNowMs();
        Render();
        if (runtimeProfile) {
            const double profileEnd = ProfileNowMs();
            const PlayableLevelSpec& level = gameWorld_.CurrentPlayableLevel();
            GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
            Tile playerTile = gameWorld_.maze.TileFromWorld(world.playerPosition.x, world.playerPosition.z);
            float monsterDistance = -1.0f;
            bool monsterVisible = false;
            if (MonsterActiveForCurrentMode()) {
                float mdx = world.monsterPosition.x - world.playerPosition.x;
                float mdz = world.monsterPosition.z - world.playerPosition.z;
                monsterDistance = std::sqrt(mdx * mdx + mdz * mdz);
                monsterVisible = world.monsterChasingVisible || world.monsterCanSeePlayerNow;
            }
            std::wostringstream csv;
            csv << std::fixed << std::setprecision(3)
                << gpuProfileRuntime_.frameCounter << L","
                << static_cast<int>(sessionRuntime_.mode) << L","
                << (profileEnd - profileStart) << L","
                << (profileAfterProgress - profileStart) << L","
                << (profileAfterBudget - profileAfterProgress) << L","
                << (profileAfterSimulation - profileAfterBudget) << L","
                << (profileAfterAudio - profileAfterSimulation) << L","
                << (profileEnd - profileAfterAudio) << L","
                << staticSceneGeometry_.indexCount << L","
                << staticSceneGeometry_.instancedIndexCount << L","
                << staticSceneGeometry_.instancedInstanceCount << L","
                << staticSceneGeometry_.floorCeilingIndexCount << L","
                << staticSceneGeometry_.waterIndexCount << L","
                << staticSceneGeometry_.transparentIndexCount << L","
                << dynamicGeometry_.opaqueVertexCount << L","
                << dynamicGeometry_.transparentVertexCount << L","
                << effectRuntime_.airParticles.size() << L","
                << effectRuntime_.sparks.size() << L","
                << effectRuntime_.steam.size() << L","
                << effectRuntime_.runtimeLamps.size() << L","
                << level.levelInLayer << L","
                << (gameWorld_.PlayableLevelRunning() ? 1 : 0) << L","
                << (gameWorld_.PlayableScoreScreenActive() ? 1 : 0) << L","
                << (gameWorld_.exitTransitionActive ? 1 : 0) << L","
                << (level.bossEncounter ? 1 : 0) << L","
                << gameWorld_.RunSeconds() << L","
                << gameWorld_.CurrentPlayableLevelSeconds() << L","
                << benchmarkRuntime_.autoplayTimer << L","
                << playerTile.x << L","
                << playerTile.y << L","
                << monsterDistance << L","
                << (monsterVisible ? 1 : 0) << L","
                << (gameWorld_.deathActive ? 1 : 0);
            RuntimeProfileFrameLine(csv.str());
        }
    }
