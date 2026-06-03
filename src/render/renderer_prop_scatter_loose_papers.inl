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
