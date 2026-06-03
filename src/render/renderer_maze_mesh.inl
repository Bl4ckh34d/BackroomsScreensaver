// Maze mesh construction and placement.
// Included inside Renderer private section after static mesh helpers.

    void CreateMazeMesh() {
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        if (!world.maze) return;
        const Maze& maze = *world.maze;
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        std::vector<uint32_t> waterIndices;
        std::vector<uint32_t> liquidIndices;
        std::vector<uint32_t> transparentIndices;
        std::vector<uint32_t> propShadowIndices;
        vertices.reserve(maze.w * maze.h * 64);
        indices.reserve(maze.w * maze.h * 96);
        waterIndices.reserve(maze.w * maze.h * 6);
        liquidIndices.reserve(maze.w * maze.h * 18);
        transparentIndices.reserve(maze.w * maze.h * 12);
        propShadowIndices.reserve(maze.w * maze.h * 24);
        ResetMazeMeshBuildRuntime();

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

        std::vector<FloorFootprint> floorReservations;
        floorReservations.reserve(512);

        std::vector<Vertex> instancedVertices;
        std::vector<uint32_t> instancedIndices;
        std::vector<StaticInstanceData> instancedInstanceData;
        std::vector<PendingStaticInstance> pendingStaticInstances;
        std::vector<InstancedMeshRange> instancedMeshRanges;
        instancedVertices.reserve(4096);
        instancedIndices.reserve(4096);
        pendingStaticInstances.reserve(maze.w * maze.h / 2);

        StaticPropPlacementBuildContext propBuild{
            vertices,
            indices,
            transparentIndices,
            floorReservations,
            instancedVertices,
            instancedIndices,
            pendingStaticInstances,
            instancedMeshRanges,
            surfaceBuild,
            tileAvg,
            tileMin
        };

        AddDebugPropInspectionModel(vertices, indices, propShadowIndices, surfaceBuild);

        StaticPropMesh loosePaperInstancedMesh = BuildLoosePaperInstancedMesh();

        AddExitDoorPropGeometry(propBuild, exitPortal);

        float paperDensity = std::clamp(settingsRuntime_.live.paperDensity, 0.0f, 4.0f);
        float hallwayPaperDensity = std::clamp(settingsRuntime_.live.hallwayPaperRunDensity, 0.0f, 4.0f);
        float chairChance = std::min(1.0f, 0.030f * std::clamp(settingsRuntime_.live.chairDensity, 0.0f, 4.0f));
        float loosePaperChance = std::min(1.0f, 0.082f * paperDensity);
        float paperHallwayChance = std::min(1.0f, 0.13f * hallwayPaperDensity);
        float ventChance = sessionRuntime_.mode == RendererRuntimeMode::MainMenu
            ? 0.0f
            : (gEffectDebugViewer && gDebugSliceEffect == DebugSliceEffect::AirVents ? 1.0f : 0.026f);
        float waterDamageChance = 0.0f;
        float waterLikeDamageChance = std::min(1.0f, 0.003f * std::clamp(settingsRuntime_.live.waterDamageDensity, 0.0f, 4.0f));

        constexpr float kWaterFloorLift = 0.008f;
        float waterCeilingY = wallH - 0.020f;
        std::vector<WaterTileSurface> floorWaterTiles(static_cast<size_t>(maze.w * maze.h));
        std::vector<WaterTileSurface> ceilingWaterTiles(static_cast<size_t>(maze.w * maze.h));
        std::vector<PendingWallWaterPool> pendingWallWaterPools;
        pendingWallWaterPools.reserve(128);
        WaterSurfaceBuildContext waterBuild{
            vertices,
            waterIndices,
            floorWaterTiles,
            ceilingWaterTiles,
            pendingWallWaterPools,
            surfaceBuild,
            tileMin,
            kWaterFloorLift,
            waterCeilingY,
            wallH
        };
        for (int y = 1; y < maze.h - 1; ++y) {
            for (int x = 1; x < maze.w - 1; ++x) {
                if (!maze.IsOpen(x, y)) continue;
                Tile t{x, y};
                if (!gEffectDebugViewer && (t == maze.start || t == maze.exit)) continue;
                AddTileAmbientScatterProps(propBuild, loosePaperInstancedMesh, t,
                    paperDensity, hallwayPaperDensity, chairChance, loosePaperChance, paperHallwayChance, ventChance);
            }
        }
        EmitWaterSurfaceDamagePlacement(waterBuild, waterDamageChance);

        std::vector<Tile> openTiles = CollectOpenPlacementTiles();

        if (!openTiles.empty()) {
            uint32_t scatterSeed = sessionRuntime_.runtimeSeed ^ 0x61c88647u;

            float cabinetDensity = std::clamp(settingsRuntime_.live.metalCabinetDensity, 0.0f, 4.0f);
            AddMetalCabinetScatterProps(propBuild, openTiles, scatterSeed, cabinetDensity);

            bool emitWaterLiquid = false;
            LiquidDamageCoverage liquidDamageCoverage;
            LiquidCeilingFootprintReservations liquidCeilingReservations;
            constexpr float kLiquidFloorReservationPad = 0.010f;

            std::vector<LiquidCanvasSurface> floorBloodCanvas(static_cast<size_t>(maze.w * maze.h));
            std::vector<LiquidCanvasSurface> ceilingBloodCanvas(static_cast<size_t>(maze.w * maze.h));
            std::vector<LiquidCanvasSurface> floorWaterCanvas(static_cast<size_t>(maze.w * maze.h));
            std::vector<LiquidCanvasSurface> ceilingWaterCanvas(static_cast<size_t>(maze.w * maze.h));
            std::vector<WallLiquidCanvasSurface> wallWaterCanvas(static_cast<size_t>(maze.w * maze.h * 4));
            LiquidCanvasBuildContext liquidCanvasBuild{
                vertices,
                liquidIndices,
                floorBloodCanvas,
                ceilingBloodCanvas,
                floorWaterCanvas,
                ceilingWaterCanvas,
                wallWaterCanvas,
                surfaceBuild,
                kBloodFloorDecalLift,
                kWaterFloorLift + 0.0015f,
                wallH - kBloodCeilingDecalInset,
                wallH
            };

            std::vector<PendingLiquidFloorSeam> pendingLiquidFloorSeams;
            pendingLiquidFloorSeams.reserve(128);

            // The old water pass reused blood leak geometry and could mix water walls with
            // blood floor/ceiling responses. Keep water on the dedicated water-like path.

            if (settingsRuntime_.live.waterDamageEnabled) {
                EmitWaterLikeDamagePlacement(liquidCanvasBuild, floorReservations, liquidCeilingReservations,
                    liquidDamageCoverage, openTiles, waterLikeDamageChance, scatterSeed,
                    tileW, tileD, tileMin, kLiquidFloorReservationPad, wallH);
            }

            EmitBloodDamagePlacementBatch(waterBuild, liquidCanvasBuild, floorReservations, liquidCeilingReservations,
                pendingLiquidFloorSeams, liquidDamageCoverage, openTiles, scatterSeed,
                tileAvg, tileW, tileD, tileMin, kLiquidFloorReservationPad, emitWaterLiquid);

            EmitMergedLiquidFloorSeams(liquidCanvasBuild, pendingLiquidFloorSeams);
            EmitWaterWallCanvasRuns(liquidCanvasBuild);
            EmitLiquidCanvasTiles(liquidCanvasBuild);

            float roomClutterDensity = std::clamp(settingsRuntime_.live.chairDensity * 0.85f, 0.0f, 4.0f);
            AddRoomClutterScatterProps(propBuild, openTiles, scatterSeed, roomClutterDensity);
            AddLoosePaperScatterProps(propBuild, loosePaperInstancedMesh, openTiles, scatterSeed, paperDensity);
            AddHallwayPaperRunScatterProps(propBuild, loosePaperInstancedMesh, openTiles, scatterSeed, hallwayPaperDensity);
        }

        AddMazeCeilingLamps(vertices, indices, surfaceBuild);
        AddMazeFloorCeilingSurfaces(vertices, indices, surfaceBuild);
        ChunkStaticSceneIndices(vertices, indices, liquidIndices, transparentIndices, propShadowIndices);
        BuildStaticInstanceChunks(pendingStaticInstances, instancedMeshRanges, instancedIndices, instancedInstanceData);
        StartupProfileStaticSceneGeometry(vertices, indices, instancedVertices, instancedIndices, instancedInstanceData);
        CreateStaticSceneBuffers(vertices, indices, instancedVertices, instancedIndices, instancedInstanceData);
    }
