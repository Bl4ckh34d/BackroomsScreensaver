    bool MonsterReachedTile(Tile t) const {
        if (!(MonsterTile() == t)) return false;
        XMFLOAT3 center = gameWorld_.maze.WorldCenter(t, 0.0f);
        float dx = center.x - gameWorld_.monster.position.x;
        float dz = center.z - gameWorld_.monster.position.z;
        float tile = gameWorld_.maze.TileMinimum();
        return dx * dx + dz * dz < tile * tile * 0.070f;
    }

    bool MonsterGoalFarEnough(Tile goal) const {
        if (!ValidMonsterTile(goal)) return false;
        XMFLOAT3 center = gameWorld_.maze.WorldCenter(goal, 0.0f);
        float dx = center.x - gameWorld_.monster.position.x;
        float dz = center.z - gameWorld_.monster.position.z;
        float minDist = std::max(gameWorld_.maze.TileMinimum() * 0.82f, 0.70f * std::clamp(settingsRuntime_.live.monsterScale, 0.35f, 1.35f));
        return dx * dx + dz * dz >= minDist * minDist;
    }
