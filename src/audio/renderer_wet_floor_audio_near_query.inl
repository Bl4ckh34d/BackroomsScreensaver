    bool IsNearWetFootstepTile(float x, float z) const {
        Tile center = gameWorld_.maze.TileFromWorld(x, z);
        for (int dy = -1; dy <= 1; ++dy) {
            for (int dx = -1; dx <= 1; ++dx) {
                if (IsWetFootstepTile({center.x + dx, center.y + dy})) return true;
            }
        }
        return false;
    }
