    bool ProbeTunnelAhead(const XMFLOAT3& primaryDir, bool hasPrimaryDir, Tile& tunnelTile) const {
        if (sessionRuntime_.mode != RendererRuntimeMode::PlayableGame || RenderMazeView().w <= 0 || RenderMazeView().h <= 0) return false;
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        std::array<XMFLOAT3, 2> dirs{};
        int dirCount = 0;
        if (hasPrimaryDir && Length3(primaryDir) > 0.001f) {
            dirs[static_cast<size_t>(dirCount++)] = Normalize3(primaryDir, Forward());
        }
        dirs[static_cast<size_t>(dirCount++)] = Forward();

        float tile = RenderMazeView().TileMinimum();
        const float distances[] = {tile * 0.18f, tile * 0.34f, tile * 0.52f, tile * 0.74f, tile * 0.96f};
        for (int d = 0; d < dirCount; ++d) {
            for (float dist : distances) {
                Tile t = RenderMazeView().TileFromWorld(world.playerPosition.x + dirs[static_cast<size_t>(d)].x * dist,
                    world.playerPosition.z + dirs[static_cast<size_t>(d)].z * dist);
                if (IsTunnelTile(t)) {
                    tunnelTile = t;
                    return true;
                }
            }
        }
        return false;
    }
