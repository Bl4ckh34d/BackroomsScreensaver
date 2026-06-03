// Player camera attention exit attention.

    bool ExitRouteNotBlockedByMonster() const {
        if (!MonsterActiveForCurrentMode()) return true;
        XMFLOAT3 exitWorld = gameWorld_.maze.WorldCenter(gameWorld_.maze.exit, gameWorld_.player.position.y);
        float ex = exitWorld.x - gameWorld_.player.position.x;
        float ez = exitWorld.z - gameWorld_.player.position.z;
        float mx = gameWorld_.monster.position.x - gameWorld_.player.position.x;
        float mz = gameWorld_.monster.position.z - gameWorld_.player.position.z;
        float exitDist = std::sqrt(ex * ex + ez * ez);
        float monsterDist = std::sqrt(mx * mx + mz * mz);
        if (exitDist < 0.2f) return true;
        if (monsterDist < 0.2f) return false;
        float alignment = (ex * mx + ez * mz) / (exitDist * monsterDist);
        return !(monsterDist < exitDist + gameWorld_.maze.TileAverage() * 0.8f && alignment > 0.66f);
    }
