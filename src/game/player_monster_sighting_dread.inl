    void UpdateMonsterSightDread(float dt, bool threat, float monsterDist) {
        if (!settingsRuntime_.live.dreadEnabled) return;
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        viewRuntime_.monsterSightDreadCooldown = std::max(0.0f, viewRuntime_.monsterSightDreadCooldown - dt);
        bool spotted = threat && PlayerLooksAt({world.monsterPosition.x, 1.18f, world.monsterPosition.z}, MonsterSightDistance(), 0.38f);
        if (spotted && !viewRuntime_.monsterSpottedLast && viewRuntime_.monsterSightDreadCooldown <= 0.0f) {
            float proximity = Clamp01((settingsRuntime_.live.dreadMonsterDistance - monsterDist) / std::max(0.1f, settingsRuntime_.live.dreadMonsterDistance));
            float spike = std::max(settingsRuntime_.live.dreadJumpscareGain * 1.08f, 0.36f + proximity * 0.24f);
            AddDread(spike);
            viewRuntime_.flashlightAgitation = std::max(viewRuntime_.flashlightAgitation, 0.88f);
            viewRuntime_.monsterSightDreadCooldown = RandRange(5.5f, 8.5f);
        }
        viewRuntime_.monsterSpottedLast = spotted;
    }
