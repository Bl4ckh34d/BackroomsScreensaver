    void MarkWaterTile(WaterSurfaceBuildContext& build,
                       Tile t,
                       bool ceiling,
                       int side,
                       int mode,
                       float seed,
                       float score,
                       bool suppressCard = false) {
        if (!RenderMazeView().IsOpen(t.x, t.y) ||
            (!gEffectDebugViewer && (t == RenderMazeView().start || t == RenderMazeView().exit))) {
            return;
        }
        WaterTileSurface& surface = (ceiling ? build.ceilingWaterTiles : build.floorWaterTiles)[WaterTileIndex(t)];
        side = std::clamp(side, 0, 3);
        mode = std::clamp(mode, 0, 3);
        if (ceiling) {
            MarkWetCeilingTile(t);
        } else {
            MarkWetFootstepTile(t);
        }
        if (!surface.active) {
            surface.active = true;
            surface.suppressCard = suppressCard;
            surface.side = side;
            surface.mode = mode;
            surface.seed = seed;
            surface.score = score;
            return;
        }
        surface.suppressCard = surface.suppressCard && suppressCard;
        surface.mode = MergeWaterMode(surface.mode, mode);
        if (score >= surface.score) {
            surface.side = side;
            surface.seed = seed;
            surface.score = score;
        }
    }
