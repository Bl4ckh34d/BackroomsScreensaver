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
