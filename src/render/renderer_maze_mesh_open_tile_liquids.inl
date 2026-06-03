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
