    bool ActivePathValid(Tile cur) const {
        if (cameraRuntime_.pathIndex >= cameraRuntime_.path.size()) return false;
        if (!RenderMazeView().IsOpen(cur.x, cur.y)) return false;
        Tile target = cameraRuntime_.path[cameraRuntime_.pathIndex];
        if (!RenderMazeView().IsOpen(target.x, target.y)) return false;
        if (cur == target) return true;
        if (cameraRuntime_.pathIndex == 0 || !(cur == cameraRuntime_.path[cameraRuntime_.pathIndex - 1])) return false;
        return std::abs(target.x - cur.x) + std::abs(target.y - cur.y) == 1;
    }

    bool ActivePathValidForMode(Tile cur, bool freeRun) const {
        if (ActivePathValid(cur)) return true;
        if (!freeRun || cameraRuntime_.pathIndex >= cameraRuntime_.path.size() || !RenderMazeView().IsOpen(cur.x, cur.y)) return false;
        Tile target = cameraRuntime_.path[cameraRuntime_.pathIndex];
        if (!OpenAreaAllowsFreeRun(cur) || !OpenAreaAllowsFreeRun(target)) return false;
        return RenderMazeView().LineClear(cur, target);
    }
