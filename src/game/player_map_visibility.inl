// Minimap visit tracking and reveal helpers. 
// Included inside Renderer's private section from player_camera_movement.inl.

    size_t TileIndex(Tile t) const {
        return gameWorld_.TileIndex(t);
    }

    uint16_t VisitCount(Tile t) const {
        return gameWorld_.VisitCount(t);
    }

    void MarkVisited(Tile t) {
        gameWorld_.MarkVisited(t);
    }

    void RevealVisibleMapTiles() {
        Tile cameraTile = CameraTile();
        MarkVisited(cameraTile);
        if (sessionRuntime_.mode != RendererRuntimeMode::PlayableGame || !gameWorld_.HasVisitedMapTiles()) return;

        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        const Maze& maze = *world.maze;
        XMFLOAT3 forward = Normalize3(Forward(), {0.0f, 0.0f, 1.0f});
        float maxDistance = std::max(maze.TileMinimum(), settingsRuntime_.live.fogEndMeters);
        float maxDistanceSq = maxDistance * maxDistance;
        float halfConeRadians = 52.0f * kPi / 180.0f;
        float baseCos = std::cos(halfConeRadians);
        XMFLOAT3 eye{world.playerPosition.x, 0.0f, world.playerPosition.z};
        float tileRadius = maze.TileAverage() * 0.50f;

        for (int y = 0; y < maze.h; ++y) {
            for (int x = 0; x < maze.w; ++x) {
                Tile t{x, y};
                if (!maze.IsOpen(t.x, t.y)) continue;
                XMFLOAT3 center = maze.WorldCenter(t, 0.0f);
                XMFLOAT3 toTile{center.x - eye.x, 0.0f, center.z - eye.z};
                float distSq = toTile.x * toTile.x + toTile.z * toTile.z;
                if (distSq > maxDistanceSq) continue;
                float dist = std::sqrt(std::max(0.0f, distSq));
                if (dist <= tileRadius) {
                    MarkVisited(t);
                    continue;
                }
                XMFLOAT3 dir{toTile.x / dist, 0.0f, toTile.z / dist};
                float angularPadding = std::atan2(tileRadius, std::max(tileRadius, dist));
                float requiredCos = std::cos(halfConeRadians + angularPadding);
                if (Dot3(forward, dir) < std::min(baseCos, requiredCos)) continue;
                if (!maze.LineClear(cameraTile, t)) continue;
                MarkVisited(t);
            }
        }
    }
