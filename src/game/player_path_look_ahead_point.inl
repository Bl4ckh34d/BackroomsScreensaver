    XMFLOAT3 PathLookAheadPoint(float lookAheadTiles) const {
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        XMFLOAT3 current{world.playerPosition.x, world.playerPosition.y, world.playerPosition.z};
        if (cameraRuntime_.pathIndex >= cameraRuntime_.path.size()) return current;

        float tile = std::max(RenderMazeView().TileMinimum(), 0.1f);
        float remaining = std::max(tile * 0.08f, std::max(0.0f, lookAheadTiles) * tile);
        XMFLOAT3 previous = current;
        size_t index = cameraRuntime_.pathIndex;
        Tile cameraTile = CameraTile();
        if (index < cameraRuntime_.path.size() && cameraRuntime_.path[index] == cameraTile && index + 1 < cameraRuntime_.path.size()) {
            ++index;
        }

        for (; index < cameraRuntime_.path.size(); ++index) {
            XMFLOAT3 center = RenderMazeView().WorldCenter(cameraRuntime_.path[index], world.playerPosition.y);
            float dx = center.x - previous.x;
            float dz = center.z - previous.z;
            float segment = std::sqrt(dx * dx + dz * dz);
            if (segment <= 0.001f) {
                previous = center;
                continue;
            }
            if (remaining <= segment) {
                float t = remaining / segment;
                return {Lerp(previous.x, center.x, t), world.playerPosition.y, Lerp(previous.z, center.z, t)};
            }
            remaining -= segment;
            previous = center;
        }
        return previous;
    }
