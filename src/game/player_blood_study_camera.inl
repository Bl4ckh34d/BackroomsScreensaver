    Tile FindBloodStudyTile() const {
        const Maze& maze = RenderMazeView();
        auto hasWallWithOpenBack = [&](Tile t) {
            if (!maze.IsOpen(t.x, t.y)) return false;
            return (!maze.IsOpen(t.x, t.y - 1) && maze.IsOpen(t.x, t.y + 1)) ||
                (!maze.IsOpen(t.x, t.y + 1) && maze.IsOpen(t.x, t.y - 1)) ||
                (!maze.IsOpen(t.x - 1, t.y) && maze.IsOpen(t.x + 1, t.y)) ||
                (!maze.IsOpen(t.x + 1, t.y) && maze.IsOpen(t.x - 1, t.y));
        };
        for (int y = 1; y < maze.h - 1; ++y) {
            for (int x = 1; x < maze.w - 1; ++x) {
                Tile t{x, y};
                if (hasWallWithOpenBack(t)) return t;
            }
        }
        for (int y = 0; y < maze.h; ++y) {
            for (int x = 0; x < maze.w; ++x) {
                if (maze.IsOpen(x, y)) return {x, y};
            }
        }
        return maze.start;
    }

    void ApplyBloodStudyCamera() {
        const Maze& maze = RenderMazeView();
        scareRuntime_.bloodStudyTile = FindBloodStudyTile();
        XMFLOAT3 c = maze.WorldCenter(scareRuntime_.bloodStudyTile, 0.0f);
        int side = 0;
        struct StudySide { int side; Tile wall; Tile back; };
        StudySide sides[] = {
            {0, {scareRuntime_.bloodStudyTile.x, scareRuntime_.bloodStudyTile.y - 1}, {scareRuntime_.bloodStudyTile.x, scareRuntime_.bloodStudyTile.y + 1}},
            {1, {scareRuntime_.bloodStudyTile.x, scareRuntime_.bloodStudyTile.y + 1}, {scareRuntime_.bloodStudyTile.x, scareRuntime_.bloodStudyTile.y - 1}},
            {2, {scareRuntime_.bloodStudyTile.x - 1, scareRuntime_.bloodStudyTile.y}, {scareRuntime_.bloodStudyTile.x + 1, scareRuntime_.bloodStudyTile.y}},
            {3, {scareRuntime_.bloodStudyTile.x + 1, scareRuntime_.bloodStudyTile.y}, {scareRuntime_.bloodStudyTile.x - 1, scareRuntime_.bloodStudyTile.y}}
        };
        for (const StudySide& candidate : sides) {
            if (!maze.IsOpen(candidate.wall.x, candidate.wall.y) && maze.IsOpen(candidate.back.x, candidate.back.y)) {
                side = candidate.side;
                break;
            }
        }
        XMFLOAT3 forward{0.0f, 0.0f, -1.0f};
        float yaw = kPi;
        if (side == 0) {
            yaw = kPi;
            forward = {0.0f, 0.0f, -1.0f};
        } else if (side == 1) {
            yaw = 0.0f;
            forward = {0.0f, 0.0f, 1.0f};
        } else if (side == 2) {
            yaw = -kPi * 0.5f;
            forward = {-1.0f, 0.0f, 0.0f};
        } else {
            yaw = kPi * 0.5f;
            forward = {1.0f, 0.0f, 0.0f};
        }
        float axisLength = (side == 0 || side == 1) ? maze.tileD : maze.tileW;
        XMFLOAT3 position = Add3({c.x, 1.08f, c.z}, Scale3(forward, -axisLength * 0.86f));
        XMFLOAT3 wallTarget = Add3({c.x, settingsRuntime_.live.wallHeightMeters * 0.52f, c.z}, Scale3(forward, axisLength * 0.50f));
        float dx = wallTarget.x - position.x;
        float dy = wallTarget.y - position.y;
        float dz = wallTarget.z - position.z;
        float pitch = std::clamp(std::atan2(dy, std::max(0.001f, std::sqrt(dx * dx + dz * dz))), -0.10f, 0.18f);
        ApplyDebugCameraPose(position, yaw, pitch, false);
    }
