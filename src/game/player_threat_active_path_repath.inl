    bool ActiveThreatPathShouldRepath(Tile cur, Tile monsterTile) const {
        if (cameraRuntime_.pathIndex >= cameraRuntime_.path.size()) return false;

        std::vector<Tile> remaining;
        remaining.reserve(std::min<size_t>(cameraRuntime_.path.size() - cameraRuntime_.pathIndex + 1, 10));
        remaining.push_back(cur);
        size_t start = cameraRuntime_.pathIndex;
        while (start < cameraRuntime_.path.size() && cameraRuntime_.path[start] == cur) {
            ++start;
        }
        for (size_t i = start; i < cameraRuntime_.path.size() && remaining.size() < 10; ++i) {
            if (remaining.back() == cameraRuntime_.path[i]) continue;
            remaining.push_back(cameraRuntime_.path[i]);
        }
        if (remaining.size() < 2) return false;
        if (ThreatPathMovesTowardMonster(remaining, monsterTile)) return true;
        return HasSaferImmediateFleeStep(cur, monsterTile) &&
            FleeStepRiskyTowardMonster(cur, remaining[1], monsterTile);
    }
