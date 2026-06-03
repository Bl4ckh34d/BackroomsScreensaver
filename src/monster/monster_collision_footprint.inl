    bool MonsterFootprintOpen(const XMFLOAT3& pos) const {
        float visualScale = std::clamp(settingsRuntime_.live.monsterScale, 0.35f, 1.35f);
        float radius = std::clamp(0.46f * visualScale, 0.22f, 0.68f);
        const XMFLOAT2 samples[] = {
            {0.0f, 0.0f},
            { radius, 0.0f},
            {-radius, 0.0f},
            {0.0f,  radius},
            {0.0f, -radius},
            { radius * 0.54f,  radius * 0.54f},
            {-radius * 0.54f,  radius * 0.54f},
            { radius * 0.54f, -radius * 0.54f},
            {-radius * 0.54f, -radius * 0.54f}
        };
        for (const XMFLOAT2& s : samples) {
            Tile tile = gameWorld_.maze.TileFromWorld(pos.x + s.x, pos.z + s.y);
            if (!gameWorld_.maze.IsOpen(tile.x, tile.y)) return false;
        }
        return true;
    }
