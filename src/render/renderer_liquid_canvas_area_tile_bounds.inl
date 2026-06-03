        int x0 = std::clamp(static_cast<int>(std::floor((minX - build.surface.ox) / build.surface.tileW)) - 1,
            0, std::max(0, RenderMazeView().w - 1));
        int x1 = std::clamp(static_cast<int>(std::floor((maxX - build.surface.ox) / build.surface.tileW)) + 1,
            0, std::max(0, RenderMazeView().w - 1));
        int y0 = std::clamp(static_cast<int>(std::floor((minZ - build.surface.oz) / build.surface.tileD)) - 1,
            0, std::max(0, RenderMazeView().h - 1));
        int y1 = std::clamp(static_cast<int>(std::floor((maxZ - build.surface.oz) / build.surface.tileD)) + 1,
            0, std::max(0, RenderMazeView().h - 1));
