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
