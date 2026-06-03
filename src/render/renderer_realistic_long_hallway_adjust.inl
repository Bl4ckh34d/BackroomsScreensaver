    bool AdjustLongHallwayPlacement(const std::vector<FloorFootprint>& reservations,
                                    float& px,
                                    float& pz,
                                    float& yaw,
                                    float width,
                                    float depth,
                                    float seed,
                                    float tileW,
                                    float tileD,
                                    float tileMin) const {
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        if (!world.maze) return false;
        const Maze& maze = *world.maze;
        Tile t = maze.TileFromWorld(px, pz);
        if (!ConstrainedHallwayTile(t)) return true;
        XMFLOAT3 center = maze.WorldCenter(t, 0.0f);
        const Tile dirs[] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
        float bestScore = -1.0e9f;
        float bestX = px;
        float bestZ = pz;
        float bestYaw = yaw;
        for (Tile d : dirs) {
            if (!maze.IsOpen(t.x + d.x, t.y + d.y)) continue;
            float dirYaw = std::atan2(static_cast<float>(d.x), static_cast<float>(d.y));
            float jitter = (LampHash(center.x + seed * 4.9f + d.x, center.z - seed * 6.3f + d.y) - 0.5f) * 0.18f;
            const float shifts[] = {0.18f, 0.06f};
            const float flips[] = {0.0f, kPi};
            for (float shift : shifts) {
                for (float flip : flips) {
                    float cx = center.x + static_cast<float>(d.x) * tileW * shift;
                    float cz = center.z + static_cast<float>(d.y) * tileD * shift;
                    float candidateYaw = dirYaw + flip + jitter;
                    if (!LongFloorFootprintClear(reservations, cx, cz, width, depth, candidateYaw, tileMin, 0.075f)) continue;
                    float score = shift * 3.0f + static_cast<float>(maze.LocalOpenCount(maze.TileFromWorld(cx, cz), 1)) * 0.20f;
                    if (score > bestScore) {
                        bestScore = score;
                        bestX = cx;
                        bestZ = cz;
                        bestYaw = candidateYaw;
                    }
                }
            }
        }
        if (bestScore <= -1.0e8f) return false;
        px = bestX;
        pz = bestZ;
        yaw = bestYaw;
        return true;
    }
