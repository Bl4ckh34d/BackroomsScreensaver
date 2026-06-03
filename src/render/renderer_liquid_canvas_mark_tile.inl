    bool MarkLiquidCanvasTile(LiquidCanvasBuildContext& build,
                              Tile tile,
                              bool water,
                              bool ceiling,
                              uint32_t sourceMask,
                              bool centerSource,
                              bool downstream,
                              float seed,
                              float score) {
        if (!RenderMazeView().IsOpen(tile.x, tile.y) ||
            (!gEffectDebugViewer && (tile == RenderMazeView().start || tile == RenderMazeView().exit))) {
            return false;
        }
        size_t idx = WaterTileIndex(tile);
        std::vector<LiquidCanvasSurface>& canvas = LiquidCanvasVector(build, water, ceiling);
        if (idx >= canvas.size()) return false;
        LiquidCanvasSurface& surface = canvas[idx];
        surface.active = true;
        surface.sourceMask |= sourceMask & 0x0fu;
        surface.centerSource = surface.centerSource || centerSource;
        surface.downstream = surface.downstream || downstream;
        if (score >= surface.score) {
            surface.seed = seed;
            surface.score = score;
        }
        if (ceiling) {
            MarkWetCeilingTile(tile);
        } else {
            MarkWetFootstepTile(tile);
        }
        return true;
    }
