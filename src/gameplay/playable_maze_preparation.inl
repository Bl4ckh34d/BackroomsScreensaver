    void PrepareNextPlayableMaze() {
        if (nextMazePreparation_ || !gameWorld_.CanContinueAfterScoreScreen() ||
            !d3dRuntime_.device || BenchmarkDemoEnabled()) return;

        auto next = std::make_unique<NextMazePreparation>();
        next->level = RollPlayableLevel(gameWorld_.NextLevelAfterScoreScreen());
        next->flickerRoll = next->level.NeedsLampFlickerRoll() ? UnitRandom() : 0.0f;
        try {
            auto builder = std::make_unique<Renderer>();
            // Only the thread-safe D3D device is shared. All maze, geometry, and
            // effect placement state belong to the builder until publication.
            builder->d3dRuntime_.device = d3dRuntime_.device;
            builder->renderAssets_ = renderAssets_;
            builder->sessionRuntime_.mode = sessionRuntime_.mode;
            builder->sessionRuntime_.runtimeSeed = sessionRuntime_.runtimeSeed;
            builder->sessionRuntime_.rng = sessionRuntime_.rng;
            builder->gameWorld_.progressionEnabled = gameWorld_.progressionEnabled;
            builder->gameWorld_.playableRun = gameWorld_.playableRun;
            builder->gameWorld_.BeginPlayableLevel(next->level);
            next->level.ApplyRuntimeSettings(builder->settingsRuntime_.live,
                sessionRuntime_.gameplaySettings, next->flickerRoll,
                gameWorld_.DarkLayerOneAppliesTo(next->level.layer));

            next->scene = std::async(std::launch::async, [builder = std::move(builder)]() mutable {
                GameWorldMazeGenerationRequest request{};
                request.layout = MakeMazeLayoutSpec(builder->settingsRuntime_.live);
                request.generation = MakeMazeGenerationSpec(builder->settingsRuntime_.live);
                request.runtimeSeed = builder->sessionRuntime_.runtimeSeed;
                builder->gameWorld_.GenerateMaze(request);
                if (!builder->CreateMazeMaskTexture()) return std::unique_ptr<Renderer>{};
                builder->CreateMazeMesh();
                if (!builder->renderBuffers_.vertexBuffer || !builder->renderBuffers_.indexBuffer ||
                    !builder->renderBuffers_.monsterBuffer || !builder->renderBuffers_.dynamicBuffer ||
                    !builder->renderBuffers_.overlayBuffer || !builder->runtimeTextures_.lampDamageSrv ||
                    !builder->runtimeTextures_.bakedLampLightSrv || !builder->mazeRespawnState_) {
                    return std::unique_ptr<Renderer>{};
                }
                if (builder->staticSceneGeometry_.instancedInstanceCount > 0 &&
                    (!builder->renderBuffers_.instancedVertexBuffer || !builder->renderBuffers_.instancedIndexBuffer ||
                     !builder->renderBuffers_.instancedInstanceBuffer)) {
                    return std::unique_ptr<Renderer>{};
                }
                return std::move(builder);
            });
        } catch (const std::exception&) {
            StartupProfileLine(L"Could not start next maze preparation");
        }
        nextMazePreparation_ = std::move(next);
    }

    void ActivatePreparedMaze(Renderer& prepared) {
        gameWorld_.maze = std::move(prepared.gameWorld_.maze);
        staticSceneGeometry_ = std::move(prepared.staticSceneGeometry_);
        prepared.renderBuffers_.constantBuffer = std::move(renderBuffers_.constantBuffer);
        renderBuffers_ = std::move(prepared.renderBuffers_);
        runtimeTextures_.mazeSrv = std::move(prepared.runtimeTextures_.mazeSrv);
        runtimeTextures_.lampDamageTexture = std::move(prepared.runtimeTextures_.lampDamageTexture);
        runtimeTextures_.lampDamageSrv = std::move(prepared.runtimeTextures_.lampDamageSrv);
        runtimeTextures_.bakedLampLightTexture = std::move(prepared.runtimeTextures_.bakedLampLightTexture);
        runtimeTextures_.bakedLampLightSrv = std::move(prepared.runtimeTextures_.bakedLampLightSrv);
        effectRuntime_ = std::move(prepared.effectRuntime_);
        exitDoorPresentation_ = prepared.exitDoorPresentation_;
        cameraRuntime_.propLookPoints = std::move(prepared.cameraRuntime_.propLookPoints);
        mazeRespawnState_ = std::move(prepared.mazeRespawnState_);
        RespawnInCurrentMaze();
    }
