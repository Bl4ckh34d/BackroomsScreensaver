    bool LongFloorFootprintClear(const std::vector<FloorFootprint>& reservations,
                                 float px,
                                 float pz,
                                 float width,
                                 float depth,
                                 float yaw,
                                 float tileMin,
                                 float pad = 0.055f) const {
        if (!FloorFootprintClear(reservations, px, pz, width, depth, yaw, tileMin, pad)) return false;
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        if (!world.maze) return false;
        const Maze& maze = *world.maze;
        float c = std::cos(yaw);
        float s = std::sin(yaw);
        XMFLOAT3 right{c, 0.0f, -s};
        XMFLOAT3 forward{s, 0.0f, c};
        int steps = std::clamp(static_cast<int>(width / (tileMin * 0.30f)) + 2, 4, 22);
        for (int i = 0; i <= steps; ++i) {
            float along = (static_cast<float>(i) / static_cast<float>(steps) - 0.5f) * width;
            const float laterals[] = {-0.46f, 0.0f, 0.46f};
            for (float lateral : laterals) {
                XMFLOAT3 p = Add3({px, 0.0f, pz}, OrientedOffset(right, {0, 1, 0}, forward, along, 0.0f, lateral * depth));
                Tile tile = maze.TileFromWorld(p.x, p.z);
                if (!maze.IsOpen(tile.x, tile.y)) return false;
            }
        }
        return true;
    }
