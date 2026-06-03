// Monster tile/body spatial helper functions. 
// Included inside Renderer's private section from player_camera_movement.inl.

    Tile CameraTile() const {
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        return world.maze->TileFromWorld(world.playerPosition.x, world.playerPosition.z);
    }

    Tile MonsterTile() const {
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        return world.maze->TileFromWorld(world.monsterPosition.x, world.monsterPosition.z);
    }

    float MonsterBodyLengthMeters() const {
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        return std::min(10.0f, 3.9f + static_cast<float>(std::max(0, world.monsterKillCount)) * 0.75f);
    }

    float MonsterBodySpacing() const {
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        return std::max(0.10f, world.maze->TileMinimum() * 0.24f);
    }

    XMFLOAT3 MonsterTrailSample(float targetDistance) const {
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        if (monsterPresentation_.trail.empty()) return world.monsterPosition;
        XMFLOAT3 prev = monsterPresentation_.trail.front();
        float travelled = 0.0f;
        for (size_t i = 1; i < monsterPresentation_.trail.size(); ++i) {
            XMFLOAT3 next = monsterPresentation_.trail[i];
            float dx = next.x - prev.x;
            float dz = next.z - prev.z;
            float len = std::sqrt(dx * dx + dz * dz);
            if (travelled + len >= targetDistance && len > 0.001f) {
                float t = (targetDistance - travelled) / len;
                return XMFLOAT3{Lerp(prev.x, next.x, t), 0.0f, Lerp(prev.z, next.z, t)};
            }
            travelled += len;
            prev = next;
        }
        float back = std::max(0.0f, targetDistance - travelled);
        return XMFLOAT3{prev.x - std::sin(world.monsterYaw) * back, 0.0f, prev.z - std::cos(world.monsterYaw) * back};
    }

    std::vector<Tile> MonsterBodyOccupiedTiles() const {
        std::vector<Tile> occupied;
        if (!MonsterActiveForCurrentMode()) return occupied;
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        const Maze& maze = *world.maze;
        if (maze.w <= 0 || maze.h <= 0) return occupied;
        float spacing = MonsterBodySpacing();
        int samples = std::clamp(static_cast<int>(std::ceil(MonsterBodyLengthMeters() / spacing)) + 1, 4, 48);
        occupied.reserve(static_cast<size_t>(samples));
        for (int i = 0; i < samples; ++i) {
            XMFLOAT3 p = MonsterTrailSample(static_cast<float>(i) * spacing);
            Tile t = maze.TileFromWorld(p.x, p.z);
            if (!maze.IsOpen(t.x, t.y)) continue;
            bool exists = false;
            for (Tile prev : occupied) {
                if (prev == t) {
                    exists = true;
                    break;
                }
            }
            if (!exists) occupied.push_back(t);
        }
        return occupied;
    }

    bool MonsterBodyOccupiesTile(Tile tile) const {
        if (!MonsterActiveForCurrentMode()) return false;
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        const Maze& maze = *world.maze;
        if (tile == maze.start || tile == maze.exit) return false;
        if (!maze.IsOpen(tile.x, tile.y)) return false;
        for (Tile occupied : MonsterBodyOccupiedTiles()) {
            if (occupied == tile) return true;
        }
        return false;
    }
