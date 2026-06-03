    void RecordMonsterTrailPoint(const XMFLOAT3& p) {
        XMFLOAT3 head{p.x, 0.0f, p.z};
        float minStep = gameWorld_.maze.TileMinimum() * 0.040f;
        if (!monsterPresentation_.trail.empty()) {
            float dx = head.x - monsterPresentation_.trail.front().x;
            float dz = head.z - monsterPresentation_.trail.front().z;
            if (dx * dx + dz * dz < minStep * minStep) return;
        }
        monsterPresentation_.trail.insert(monsterPresentation_.trail.begin(), head);
        if (monsterPresentation_.trail.size() > 256) monsterPresentation_.trail.resize(256);
    }
