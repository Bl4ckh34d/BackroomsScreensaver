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
