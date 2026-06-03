// Monster focus-point and preview-camera helpers. 
// Included inside Renderer's private section from player_monster_pressure.inl.

    float MonsterDistance() const {
        if (!MonsterActiveForCurrentMode()) return std::numeric_limits<float>::max();
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        float dx = world.monsterPosition.x - world.playerPosition.x;
        float dz = world.monsterPosition.z - world.playerPosition.z;
        return std::sqrt(dx * dx + dz * dz);
    }
