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
