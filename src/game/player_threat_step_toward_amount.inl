    float StepTowardMonsterAmount(Tile step) const {
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        XMFLOAT3 sw = RenderMazeView().WorldCenter(step, world.playerPosition.y);
        float stepX = sw.x - world.playerPosition.x;
        float stepZ = sw.z - world.playerPosition.z;
        float monX = world.monsterPosition.x - world.playerPosition.x;
        float monZ = world.monsterPosition.z - world.playerPosition.z;
        float stepLen = std::sqrt(stepX * stepX + stepZ * stepZ);
        float monLen = std::sqrt(monX * monX + monZ * monZ);
        if (stepLen <= 0.001f || monLen <= 0.001f) return -1.0f;
        return (stepX * monX + stepZ * monZ) / (stepLen * monLen);
    }
