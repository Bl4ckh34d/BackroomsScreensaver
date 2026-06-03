    bool RecoverPlayerCollisionFootprint() {
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        Tile cur = CameraTile();
        Tile best = cur;
        float bestScore = std::numeric_limits<float>::infinity();
        if (!RenderMazeView().IsOpen(best.x, best.y)) {
            for (int y = 1; y < RenderMazeView().h - 1; ++y) {
                for (int x = 1; x < RenderMazeView().w - 1; ++x) {
                    if (!RenderMazeView().IsOpen(x, y)) continue;
                    XMFLOAT3 c = RenderMazeView().WorldCenter({x, y}, world.playerPosition.y);
                    float dx = c.x - world.playerPosition.x;
                    float dz = c.z - world.playerPosition.z;
                    float score = dx * dx + dz * dz;
                    if (score < bestScore) {
                        bestScore = score;
                        best = {x, y};
                    }
                }
            }
        }
        if (!RenderMazeView().IsOpen(best.x, best.y)) return false;

        XMFLOAT3 center = RenderMazeView().WorldCenter(best, world.playerPosition.y);
        gameWorld_.SetPlayerHorizontalPosition(center.x, center.z);
        if (!(best == cameraRuntime_.lastTile) && AdjacentTiles(best, cameraRuntime_.lastTile)) {
            cameraRuntime_.previousTile = cameraRuntime_.lastTile;
        }
        cameraRuntime_.lastTile = best;
        cameraRuntime_.path.clear();
        cameraRuntime_.pathIndex = 0;
        return true;
    }

