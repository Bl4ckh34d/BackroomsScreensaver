        auto neighborWet = [&](int nx, int ny) {
            if (!RenderMazeView().IsOpen(nx, ny)) return false;
            const WaterTileSurface& neighbor =
                (ceiling ? build.ceilingWaterTiles : build.floorWaterTiles)[static_cast<size_t>(ny * RenderMazeView().w + nx)];
            return neighbor.active && !neighbor.suppressCard;
        };
        auto neighborOpen = [&](int nx, int ny) {
            return RenderMazeView().IsOpen(nx, ny);
        };
        if (neighborWet(t.x, t.y - 1)) neighborMask |= 1;
        if (neighborWet(t.x, t.y + 1)) neighborMask |= 2;
        if (neighborWet(t.x - 1, t.y)) neighborMask |= 4;
        if (neighborWet(t.x + 1, t.y)) neighborMask |= 8;
        if (neighborWet(t.x - 1, t.y - 1)) neighborMask |= 16;
        if (neighborWet(t.x + 1, t.y - 1)) neighborMask |= 32;
        if (neighborWet(t.x - 1, t.y + 1)) neighborMask |= 64;
        if (neighborWet(t.x + 1, t.y + 1)) neighborMask |= 128;
