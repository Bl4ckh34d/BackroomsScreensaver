    bool FootprintFitsMaze(float px,
                           float pz,
                           float width,
                           float depth,
                           float yaw,
                           float wallPad,
                           float tileMin) const {
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        if (!world.maze) return false;
        const Maze& maze = *world.maze;
        float c = std::cos(yaw);
        float s = std::sin(yaw);
        XMFLOAT3 right{c, 0.0f, -s};
        XMFLOAT3 forward{s, 0.0f, c};
        float hx = width * 0.5f + wallPad;
        float hz = depth * 0.5f + wallPad;
        int sxCount = std::clamp(static_cast<int>(std::ceil((width + wallPad * 2.0f) / (tileMin * 0.18f))) + 2, 3, 13);
        int szCount = std::clamp(static_cast<int>(std::ceil((depth + wallPad * 2.0f) / (tileMin * 0.18f))) + 2, 3, 13);
        for (int sy = 0; sy < szCount; ++sy) {
            float fy = szCount == 1 ? 0.0f : static_cast<float>(sy) / static_cast<float>(szCount - 1);
            float ly = Lerp(-hz, hz, fy);
            for (int sx = 0; sx < sxCount; ++sx) {
                float fx = sxCount == 1 ? 0.0f : static_cast<float>(sx) / static_cast<float>(sxCount - 1);
                float lx = Lerp(-hx, hx, fx);
                XMFLOAT3 p = Add3({px, 0.0f, pz}, OrientedOffset(right, {0, 1, 0}, forward, lx, 0.0f, ly));
                Tile tile = maze.TileFromWorld(p.x, p.z);
                if (!maze.IsOpen(tile.x, tile.y)) return false;
            }
        }
        return true;
    }
