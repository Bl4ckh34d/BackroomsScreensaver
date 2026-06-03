    bool VisibleInFront(Tile target) const {
        Tile cur = CameraTile();
        if (!gameWorld_.maze.LineClear(cur, target)) return false;
        XMFLOAT3 tw = gameWorld_.maze.WorldCenter(target, gameWorld_.player.position.y);
        XMFLOAT3 f = Forward();
        float dx = tw.x - gameWorld_.player.position.x;
        float dz = tw.z - gameWorld_.player.position.z;
        float len = std::sqrt(dx * dx + dz * dz);
        if (len < 0.01f) return true;
        return (dx * f.x + dz * f.z) / len > 0.38f;
    }
