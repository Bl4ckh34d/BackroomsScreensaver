    float AudioOcclusionFor(XMFLOAT3 source) const {
        if (monsterPreview_.active || sessionRuntime_.mode == RendererRuntimeMode::MainMenu) return 0.0f;
        float dist = DistanceToPoint(source);
        if (dist > std::max(72.0f, gameWorld_.maze.TileAverage() * 32.0f)) return 0.0f;
        XMFLOAT3 listener{gameWorld_.player.position.x, 1.30f, gameWorld_.player.position.z};
        source.y = std::clamp(source.y, 0.15f, settingsRuntime_.live.wallHeightMeters - 0.08f);
        Tile listenerTile = gameWorld_.maze.TileFromWorld(listener.x, listener.z);
        Tile sourceTile = gameWorld_.maze.TileFromWorld(source.x, source.z);
        if (listenerTile == sourceTile) return 0.0f;

        XMFLOAT3 forward = Normalize3(Sub3(source, listener), {0.0f, 0.0f, 1.0f});
        XMFLOAT3 right = Normalize3(Cross3({0.0f, 1.0f, 0.0f}, forward), {1.0f, 0.0f, 0.0f});
        float radius = std::clamp(gameWorld_.maze.TileMinimum() * 0.16f, 0.12f, 0.30f);
        std::array<float, 5> offsets{{0.0f, -1.0f, 1.0f, -0.45f, 0.45f}};
        int minBlocks = 8;
        for (float offset : offsets) {
            XMFLOAT3 l = Add3(listener, Scale3(right, offset * radius));
            XMFLOAT3 s = Add3(source, Scale3(right, -offset * radius * 0.60f));
            minBlocks = std::min(minBlocks, AudioWallBlocksBetween(l, s));
        }
        return static_cast<float>(minBlocks);
    }
