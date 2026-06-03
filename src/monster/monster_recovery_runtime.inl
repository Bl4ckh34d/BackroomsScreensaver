    bool RecoverMonsterToNearestOpenTile(Tile fromTile) {
        if (gameWorld_.maze.IsOpen(fromTile.x, fromTile.y)) return true;
        Tile best{-1000, -1000};
        float bestSq = std::numeric_limits<float>::max();
        for (int radius = 1; radius <= 5 && !ValidMonsterTile(best); ++radius) {
            for (int dz = -radius; dz <= radius; ++dz) {
                for (int dx = -radius; dx <= radius; ++dx) {
                    if (std::abs(dx) != radius && std::abs(dz) != radius) continue;
                    Tile t{fromTile.x + dx, fromTile.y + dz};
                    if (!gameWorld_.maze.IsOpen(t.x, t.y)) continue;
                    XMFLOAT3 c = gameWorld_.maze.WorldCenter(t, 0.0f);
                    float sx = c.x - gameWorld_.monster.position.x;
                    float sz = c.z - gameWorld_.monster.position.z;
                    float d2 = sx * sx + sz * sz;
                    if (d2 < bestSq) {
                        bestSq = d2;
                        best = t;
                    }
                }
            }
        }
        if (!ValidMonsterTile(best)) return false;
        XMFLOAT3 target = gameWorld_.maze.WorldCenter(best, 0.0f);
        float maxStep = std::max(gameWorld_.maze.TileMinimum() * 0.18f, settingsRuntime_.live.monsterSpeed * 0.12f);
        XMFLOAT3 delta{target.x - gameWorld_.monster.position.x, 0.0f, target.z - gameWorld_.monster.position.z};
        float len = Length3(delta);
        gameWorld_.monster.position = len > maxStep ? Add3(gameWorld_.monster.position, Scale3(delta, maxStep / std::max(0.001f, len))) : target;
        if (len > gameWorld_.maze.TileAverage() * 1.3f) {
            ResetMonsterPresentationState(true, false, false);
        }
        RecordMonsterTrailPoint(gameWorld_.monster.position);
        ClearMonsterPath();
        return true;
    }
