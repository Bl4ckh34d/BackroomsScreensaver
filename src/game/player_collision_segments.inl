    bool PlayerCollisionSegmentOpen(float fromX, float fromZ, float toX, float toZ, Tile allowedStart, Tile allowedTarget) const {
        float dx = toX - fromX;
        float dz = toZ - fromZ;
        float len = std::sqrt(dx * dx + dz * dz);
        int steps = std::max(1, static_cast<int>(std::ceil(len / std::max(0.05f, RenderMazeView().TileMinimum() * 0.08f))));
        Tile prev = RenderMazeView().TileFromWorld(fromX, fromZ);
        if (!PlayerCollisionTilePassable(prev)) return false;
        if (!(prev == allowedStart) && !(prev == allowedTarget)) return false;
        for (int i = 1; i <= steps; ++i) {
            float t = static_cast<float>(i) / static_cast<float>(steps);
            float sx = Lerp(fromX, toX, t);
            float sz = Lerp(fromZ, toZ, t);
            Tile cur = RenderMazeView().TileFromWorld(sx, sz);
            if (MonsterBodyOccupiesTile(cur) && !(cur == allowedStart)) return false;
            if (!(cur == allowedStart) && !(cur == allowedTarget)) return false;
            if (!(cur == prev)) {
                if (!PlayerCollisionTilePassable(cur)) return false;
                if (!AdjacentTiles(prev, cur)) return false;
                prev = cur;
            }
            if (!PlayerCollisionFootprintOpen(sx, sz)) return false;
        }
        return true;
    }

    bool PlayerCollisionSegmentOpenThroughOpen(float fromX, float fromZ, float toX, float toZ, bool allowDiagonal) const {
        float dx = toX - fromX;
        float dz = toZ - fromZ;
        float len = std::sqrt(dx * dx + dz * dz);
        int steps = std::max(1, static_cast<int>(std::ceil(len / std::max(0.05f, RenderMazeView().TileMinimum() * 0.07f))));
        Tile prev = RenderMazeView().TileFromWorld(fromX, fromZ);
        if (!PlayerCollisionTilePassable(prev) || !PlayerCollisionFootprintOpen(fromX, fromZ)) return false;
        for (int i = 1; i <= steps; ++i) {
            float t = static_cast<float>(i) / static_cast<float>(steps);
            float sx = Lerp(fromX, toX, t);
            float sz = Lerp(fromZ, toZ, t);
            Tile cur = RenderMazeView().TileFromWorld(sx, sz);
            if (MonsterBodyOccupiesTile(cur) && !(cur == prev)) return false;
            if (!PlayerCollisionTilePassable(cur)) return false;
            if (!(cur == prev)) {
                int stepX = cur.x - prev.x;
                int stepY = cur.y - prev.y;
                if (std::abs(stepX) > 1 || std::abs(stepY) > 1) return false;
                if (std::abs(stepX) == 1 && std::abs(stepY) == 1) {
                    if (!allowDiagonal) return false;
                    if (!PlayerCollisionTilePassable({prev.x + stepX, prev.y}) || !PlayerCollisionTilePassable({prev.x, prev.y + stepY})) return false;
                    if (!OpenAreaAllowsFreeRun(prev) || !OpenAreaAllowsFreeRun(cur)) return false;
                }
                prev = cur;
            }
            if (!PlayerCollisionFootprintOpen(sx, sz)) return false;
        }
        return true;
    }

