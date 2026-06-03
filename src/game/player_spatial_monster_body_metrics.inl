    float MonsterBodyLengthMeters() const {
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        return std::min(10.0f, 3.9f + static_cast<float>(std::max(0, world.monsterKillCount)) * 0.75f);
    }

    float MonsterBodySpacing() const {
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        return std::max(0.10f, world.maze->TileMinimum() * 0.24f);
    }
