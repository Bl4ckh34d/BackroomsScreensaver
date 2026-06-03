// Player camera attention corridor scan.

    bool StraightCorridorTravelYaw(Tile cameraTile, float& corridorYaw) const {
        corridorYaw = gameWorld_.player.yaw;
        if (!IsStraightCorridor(cameraTile) || cameraRuntime_.pathIndex >= cameraRuntime_.path.size()) return false;

        size_t startIndex = cameraRuntime_.pathIndex;
        Tile next = cameraRuntime_.path[startIndex];
        if (next == cameraTile) {
            if (startIndex + 1 >= cameraRuntime_.path.size()) return false;
            ++startIndex;
            next = cameraRuntime_.path[startIndex];
        }
        if (!AdjacentTiles(cameraTile, next)) return false;

        int dirX = next.x - cameraTile.x;
        int dirY = next.y - cameraTile.y;
        Tile previous = cameraTile;
        int straightSteps = 0;
        const size_t limit = std::min(cameraRuntime_.path.size(), startIndex + 3);
        for (size_t i = startIndex; i < limit; ++i) {
            Tile step = cameraRuntime_.path[i];
            if (!AdjacentTiles(previous, step)) return false;
            int stepX = step.x - previous.x;
            int stepY = step.y - previous.y;
            if (stepX != dirX || stepY != dirY) return false;
            previous = step;
            ++straightSteps;
        }
        if (straightSteps < 2) return false;

        XMFLOAT3 a = gameWorld_.maze.WorldCenter(cameraTile, gameWorld_.player.position.y);
        XMFLOAT3 b = gameWorld_.maze.WorldCenter(next, gameWorld_.player.position.y);
        corridorYaw = std::atan2(b.x - a.x, b.z - a.z);
        return true;
    }
