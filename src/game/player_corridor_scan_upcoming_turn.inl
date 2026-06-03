    bool FindUpcomingCorridorTurn(Tile cameraTile, float& turnYaw, float& turnWeight) const {
        turnYaw = gameWorld_.player.yaw;
        turnWeight = 0.0f;
        if (!IsCorridorLike(cameraTile) || cameraRuntime_.pathIndex >= cameraRuntime_.path.size() || cameraRuntime_.path.size() < 2) return false;

        const float tile = std::max(gameWorld_.maze.TileMinimum(), 0.1f);
        const size_t lastCandidate = std::min(cameraRuntime_.path.size() - 2, cameraRuntime_.pathIndex + 4);
        Tile previous = cameraTile;
        for (size_t i = cameraRuntime_.pathIndex; i <= lastCandidate; ++i) {
            Tile turn = cameraRuntime_.path[i];
            if (turn == previous) continue;
            if (!AdjacentTiles(previous, turn)) break;

            Tile next = cameraRuntime_.path[i + 1];
            if (!AdjacentTiles(turn, next)) break;

            int inX = turn.x - previous.x;
            int inY = turn.y - previous.y;
            int outX = next.x - turn.x;
            int outY = next.y - turn.y;
            if (inX == outX && inY == outY) {
                previous = turn;
                continue;
            }

            if (outX == -inX && outY == -inY) return false;

            XMFLOAT3 turnCenter = gameWorld_.maze.WorldCenter(turn, gameWorld_.player.position.y);
            float dx = turnCenter.x - gameWorld_.player.position.x;
            float dz = turnCenter.z - gameWorld_.player.position.z;
            float distToTurn = std::sqrt(dx * dx + dz * dz);
            float weight = SmoothStep(tile * 1.42f, tile * 0.22f, distToTurn);
            float futureFade = 1.0f - std::min(static_cast<float>(i - cameraRuntime_.pathIndex), 3.0f) * 0.16f;
            turnWeight = weight * futureFade * 0.46f;
            if (turnWeight <= 0.001f) return false;

            XMFLOAT3 nextCenter = gameWorld_.maze.WorldCenter(next, gameWorld_.player.position.y);
            turnYaw = std::atan2(nextCenter.x - turnCenter.x, nextCenter.z - turnCenter.z);
            return true;
        }
        return false;
    }
