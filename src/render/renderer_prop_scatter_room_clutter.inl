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
