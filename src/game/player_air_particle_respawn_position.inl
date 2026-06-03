    void RespawnAirParticle(AirParticle& p, bool initial) {
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        const Maze& maze = *world.maze;
        float radius = std::clamp(settingsRuntime_.live.flashlightShadowDistanceMeters * 0.70f, 6.0f, 12.0f);
        XMFLOAT3 forward{std::sin(viewRuntime_.flashlightYaw), 0.0f, std::cos(viewRuntime_.flashlightYaw)};
        XMFLOAT3 right{std::cos(viewRuntime_.flashlightYaw), 0.0f, -std::sin(viewRuntime_.flashlightYaw)};
        XMFLOAT3 pos = world.playerPosition;
        float coneHalf = std::clamp(settingsRuntime_.live.flashlightConeDegrees, 20.0f, 140.0f) * 0.5f * kPi / 180.0f;
        float coneSpread = std::tan(std::min(coneHalf * 0.72f, 1.12f));
        float layerRoll = RandRange(0.0f, 1.0f);
        p.nearLayer = layerRoll < 0.018f ? 2.0f : (layerRoll < 0.165f ? 1.0f : 0.0f);
        for (int attempt = 0; attempt < 24; ++attempt) {
            float depthT = std::pow(RandRange(0.0f, 1.0f), 0.42f);
            float depth = Lerp(0.45f, radius, depthT);
            if (p.nearLayer > 1.5f) {
                depth = RandRange(0.24f, 1.20f);
            } else if (p.nearLayer > 0.5f) {
                depth = RandRange(0.65f, 2.85f);
            } else if (RandRange(0.0f, 1.0f) < 0.10f) {
                depth = RandRange(0.30f, radius);
            }
            float sideLimit = std::clamp(depth * coneSpread, 0.22f, radius * 0.82f);
            float side = RandRange(-sideLimit, sideLimit);
            float yMin = p.nearLayer > 0.5f ? 0.34f : 0.22f;
            float yMax = std::max(yMin + 0.02f, settingsRuntime_.live.wallHeightMeters - (p.nearLayer > 0.5f ? 0.24f : 0.14f));
            float y = RandRange(yMin, yMax);
            pos = Add3({world.playerPosition.x, y, world.playerPosition.z}, Add3(Scale3(forward, depth), Scale3(right, side)));
            Tile tile = maze.TileFromWorld(pos.x, pos.z);
            if (maze.IsOpen(tile.x, tile.y)) break;
        }
