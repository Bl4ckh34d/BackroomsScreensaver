    void QueueWallWaterPoolCard(WaterSurfaceBuildContext& build,
                                Tile owner,
                                float cx,
                                float cz,
                                int side,
                                float seed,
                                float width,
                                float depth,
                                float yaw,
                                float score) {
        if (!RenderMazeView().IsOpen(owner.x, owner.y) ||
            (!gEffectDebugViewer && (owner == RenderMazeView().start || owner == RenderMazeView().exit))) {
            return;
        }
        build.pendingWallWaterPools.push_back({owner, side, cx, cz, width, depth, yaw, seed, score});
    }
