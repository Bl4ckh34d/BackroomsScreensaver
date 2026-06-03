// Static prop placement scatter.

    void AddMetalCabinetScatterProps(StaticPropPlacementBuildContext& build,
                                     const std::vector<Tile>& openTiles,
                                     uint32_t scatterSeed,
                                     float cabinetDensity) {
        int cabinetTarget = cabinetDensity <= 0.001f
            ? 0
            : std::clamp(static_cast<int>(std::round(static_cast<float>(openTiles.size()) * 0.014f * cabinetDensity)), 2, 76);
        int placedCabinets = 0;
        int cabinetAttempts = cabinetTarget * 10;
        for (int cidx = 0; cidx < cabinetAttempts && placedCabinets < cabinetTarget; ++cidx) {
            size_t tileIndex = std::min(openTiles.size() - 1,
                static_cast<size_t>(Rand01(cidx, 971, scatterSeed) * static_cast<float>(openTiles.size())));
            Tile t = openTiles[tileIndex];
            if (t == RenderMazeView().start || t == RenderMazeView().exit) continue;
            int sides[4]{};
            int sideCount = 0;
            if (!RenderMazeView().IsOpen(t.x, t.y - 1)) sides[sideCount++] = 0;
            if (!RenderMazeView().IsOpen(t.x, t.y + 1)) sides[sideCount++] = 1;
            if (!RenderMazeView().IsOpen(t.x - 1, t.y)) sides[sideCount++] = 2;
            if (!RenderMazeView().IsOpen(t.x + 1, t.y)) sides[sideCount++] = 3;
            if (sideCount == 0) continue;
            int side = sides[std::min(sideCount - 1, static_cast<int>(Rand01(cidx, 977, scatterSeed) * static_cast<float>(sideCount)))];
            if (AddMetalCabinetAgainstWallProp(build, t, side, Rand01(cidx, 983, scatterSeed))) {
                ++placedCabinets;
            }
        }
    }

    void AddRoomClutterScatterProps(StaticPropPlacementBuildContext& build,
                                    const std::vector<Tile>& openTiles,
                                    uint32_t scatterSeed,
                                    float roomClutterDensity) {
        int roomGroupMin = roomClutterDensity >= 0.72f ? 4 : (roomClutterDensity < 0.12f ? 0 : 1);
        int roomGroups = roomClutterDensity <= 0.001f
            ? 0
            : std::clamp(static_cast<int>(std::round(static_cast<float>(openTiles.size()) * 0.010f * roomClutterDensity)),
                roomGroupMin, 42);
        int roomAttempts = roomGroups * 7;
        int placedRoomGroups = 0;
        for (int g = 0; g < roomAttempts && placedRoomGroups < roomGroups; ++g) {
            size_t tileIndex = std::min(openTiles.size() - 1,
                static_cast<size_t>(Rand01(g, 277, scatterSeed) * static_cast<float>(openTiles.size())));
            Tile t = openTiles[tileIndex];
            if (!IsRoomLike(t)) continue;
            if (AddRoomClutterGroupProp(build, t, g, scatterSeed)) {
                ++placedRoomGroups;
            }
        }
    }

    void AddLoosePaperScatterProps(StaticPropPlacementBuildContext& build,
                                   const StaticPropMesh& loosePaperInstancedMesh,
                                   const std::vector<Tile>& openTiles,
                                   uint32_t scatterSeed,
                                   float paperDensity) {
        int basePageMin = paperDensity >= 0.95f ? 260 : 80;
        int basePages = std::clamp(static_cast<int>(openTiles.size() / 3), basePageMin, 900);
        int targetPages = std::clamp(static_cast<int>(basePages * paperDensity), 0, 3600);
        int attempts = targetPages * 4;
        int placed = 0;
        for (int i = 0; i < attempts && placed < targetPages; ++i) {
            size_t tileIndex = std::min(openTiles.size() - 1,
                static_cast<size_t>(Rand01(i, 17, scatterSeed) * static_cast<float>(openTiles.size())));
            Tile t = openTiles[tileIndex];
            XMFLOAT3 c = RenderMazeView().WorldCenter(t, 0.0f);
            float px = c.x + (Rand01(i, 37, scatterSeed) - 0.5f) * std::max(0.10f, build.surface.tileW - kLoosePaperLongMeters - 0.10f);
            float pz = c.z + (Rand01(i, 41, scatterSeed) - 0.5f) * std::max(0.10f, build.surface.tileD - kLoosePaperLongMeters - 0.10f);
            float yaw = Rand01(i, 43, scatterSeed) * kPi * 2.0f;
            float lift = 0.0030f + Rand01(i, 47, scatterSeed) * 0.0015f;
            float material = LoosePaperMaterial(Rand01(i, 52, scatterSeed), Rand01(i, 54, scatterSeed));
            if (AddLoosePaperProp(build, loosePaperInstancedMesh, px, pz, yaw, lift, material)) {
                if (Rand01(i, 49, scatterSeed) < 0.010f) {
                    AddCassetteProp(build, px, pz, yaw, lift + 0.003f, Rand01(i, 51, scatterSeed));
                }
                ++placed;
            }
        }
    }

    void AddHallwayPaperRunScatterProps(StaticPropPlacementBuildContext& build,
                                        const StaticPropMesh& loosePaperInstancedMesh,
                                        const std::vector<Tile>& openTiles,
                                        uint32_t scatterSeed,
                                        float hallwayPaperDensity) {
        int baseRuns = std::clamp(static_cast<int>(openTiles.size() / 220), 5, 14);
        int runs = std::clamp(static_cast<int>(std::round(baseRuns * hallwayPaperDensity)), 0, 56);
        for (int run = 0; run < runs; ++run) {
            size_t tileIndex = std::min(openTiles.size() - 1,
                static_cast<size_t>(Rand01(run, 53, scatterSeed) * static_cast<float>(openTiles.size())));
            Tile t = openTiles[tileIndex];
            bool ew = RenderMazeView().IsOpen(t.x - 1, t.y) && RenderMazeView().IsOpen(t.x + 1, t.y);
            bool ns = RenderMazeView().IsOpen(t.x, t.y - 1) && RenderMazeView().IsOpen(t.x, t.y + 1);
            if (!ew && !ns) continue;
            bool useEW = ew && (!ns || Rand01(run, 59, scatterSeed) < 0.5f);
            XMFLOAT3 c = RenderMazeView().WorldCenter(t, 0.0f);
            float runLen = (useEW ? build.surface.tileW : build.surface.tileD) * (2.4f + Rand01(run, 61, scatterSeed) * 4.8f);
            int count = std::max(1, static_cast<int>((22.0f + Rand01(run, 67, scatterSeed) * 34.0f) * hallwayPaperDensity));
            for (int p = 0; p < count; ++p) {
                float along = (Rand01(run * 97 + p, 71, scatterSeed) - 0.5f) * runLen;
                float cross = (Rand01(run * 97 + p, 73, scatterSeed) - 0.5f) * ((useEW ? build.surface.tileD : build.surface.tileW) * 0.82f);
                float px = c.x + (useEW ? along : cross);
                float pz = c.z + (useEW ? cross : along);
                float yaw = Rand01(run * 97 + p, 97, scatterSeed) * kPi * 2.0f;
                float lift = 0.0032f + p * 0.00010f + Rand01(run * 97 + p, 101, scatterSeed) * 0.0014f;
                float material = LoosePaperMaterial(Rand01(run * 97 + p, 109, scatterSeed), Rand01(run * 97 + p, 113, scatterSeed));
                if (AddLoosePaperProp(build, loosePaperInstancedMesh, px, pz, yaw, lift, material) &&
                    Rand01(run * 97 + p, 103, scatterSeed) < 0.010f) {
                    AddCassetteProp(build, px, pz, yaw, lift + 0.003f, Rand01(run * 97 + p, 107, scatterSeed));
                }
            }
            cameraRuntime_.propLookPoints.push_back({c.x, 0.16f, c.z});
        }
    }

    void AddTileAmbientScatterProps(StaticPropPlacementBuildContext& build,
                                    const StaticPropMesh& loosePaperInstancedMesh,
                                    Tile t,
                                    float paperDensity,
                                    float hallwayPaperDensity,
                                    float chairChance,
                                    float loosePaperChance,
                                    float paperHallwayChance,
                                    float ventChance) {
        XMFLOAT3 c = RenderMazeView().WorldCenter(t, 0.0f);
        float h0 = PropPlacementTileHash(t.x, t.y, 0.1f);
        float h1 = PropPlacementTileHash(t.x, t.y, 1.7f);
        float h2 = PropPlacementTileHash(t.x, t.y, 2.9f);
        bool openN = RenderMazeView().IsOpen(t.x, t.y - 1);
        bool openS = RenderMazeView().IsOpen(t.x, t.y + 1);
        bool openW = RenderMazeView().IsOpen(t.x - 1, t.y);
        bool openE = RenderMazeView().IsOpen(t.x + 1, t.y);
        bool corridorNS = openN && openS && !openW && !openE;
        bool corridorEW = openW && openE && !openN && !openS;
        auto plainWallNeighbor = [&](int nx, int ny) {
            return !RenderMazeView().IsOpen(nx, ny) && RenderMazeView().WallFeature(nx, ny) == MazeWallFeature::None;
        };

        if (h0 < chairChance && IsRoomLike(t)) {
            float chairX = c.x + (h1 - 0.5f) * std::max(0.0f, build.surface.tileW - 1.12f) * 0.42f;
            float chairZ = c.z + (h2 - 0.5f) * std::max(0.0f, build.surface.tileD - 1.12f) * 0.42f;
            float chairYaw = h1 * kPi * 2.0f;
            AddChairProp(build, {chairX, 0.0f, chairZ}, chairYaw, h2 < 0.55f);
        }

        bool paperHallway = false;
        if (corridorEW) paperHallway = PropPlacementTileHash(t.x / 4, t.y, 26.0f) < paperHallwayChance;
        if (corridorNS) paperHallway = paperHallway || PropPlacementTileHash(t.x, t.y / 4, 27.0f) < paperHallwayChance;
        if (paperHallway) {
            int count = std::max(1, static_cast<int>((11.0f + PropPlacementTileHash(t.x, t.y, 4.0f) * 18.0f) * hallwayPaperDensity));
            AddLoosePapersProp(build, loosePaperInstancedMesh, t, count, true);
        } else if (h1 < loosePaperChance) {
            int count = std::max(1, static_cast<int>((2.0f + PropPlacementTileHash(t.x, t.y, 4.0f) * 6.0f) * paperDensity));
            AddLoosePapersProp(build, loosePaperInstancedMesh, t, count, false);
        }

        int ventSides[4]{};
        int ventSideCount = 0;
        if (plainWallNeighbor(t.x, t.y - 1)) ventSides[ventSideCount++] = 0;
        if (plainWallNeighbor(t.x, t.y + 1)) ventSides[ventSideCount++] = 1;
        if (plainWallNeighbor(t.x - 1, t.y)) ventSides[ventSideCount++] = 2;
        if (plainWallNeighbor(t.x + 1, t.y)) ventSides[ventSideCount++] = 3;
        if (ventSideCount > 0 && PropPlacementTileHash(t.x, t.y, 31.0f) < ventChance) {
            int sideIndex = std::min(ventSideCount - 1, static_cast<int>(PropPlacementTileHash(t.x, t.y, 32.0f) * ventSideCount));
            AddWallVentProp(build.vertices, build.indices,
                build.instancedVertices, build.instancedIndices, build.pendingStaticInstances, build.instancedMeshRanges,
                build.surface, t, ventSides[sideIndex], PropPlacementTileHash(t.x, t.y, 33.0f));
        }
    }
