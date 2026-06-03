    bool TunnelImmediateVicinity(Tile& tunnelTile) const {
        if (sessionRuntime_.mode != RendererRuntimeMode::PlayableGame || RenderMazeView().w <= 0 || RenderMazeView().h <= 0) return false;
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        Tile cur = CameraTile();
        float ox = -static_cast<float>(RenderMazeView().w) * RenderMazeView().tileW * 0.5f;
        float oz = -static_cast<float>(RenderMazeView().h) * RenderMazeView().tileD * 0.5f;
        float margin = std::clamp(RenderMazeView().TileMinimum() * 0.22f, 0.34f, 0.58f);
        float bestDistSq = margin * margin;
        bool found = false;

        for (int y = cur.y - 1; y <= cur.y + 1; ++y) {
            for (int x = cur.x - 1; x <= cur.x + 1; ++x) {
                Tile t{x, y};
                if (!IsTunnelTile(t)) continue;
                float x0 = ox + static_cast<float>(x) * RenderMazeView().tileW;
                float x1 = x0 + RenderMazeView().tileW;
                float z0 = oz + static_cast<float>(y) * RenderMazeView().tileD;
                float z1 = z0 + RenderMazeView().tileD;
                float dx = std::max(std::max(x0 - world.playerPosition.x, 0.0f), world.playerPosition.x - x1);
                float dz = std::max(std::max(z0 - world.playerPosition.z, 0.0f), world.playerPosition.z - z1);
                float distSq = dx * dx + dz * dz;
                if (distSq <= bestDistSq) {
                    bestDistSq = distSq;
                    tunnelTile = t;
                    found = true;
                }
            }
        }
        return found;
    }
