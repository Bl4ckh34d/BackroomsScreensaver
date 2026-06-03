// Monster sight, curiosity, and awareness helper functions. 
// Included inside Renderer's private section from monster_ai.inl.

    float MonsterSightDistance() const {
        return MonsterSightDistanceMeters(settingsRuntime_.live);
    }

    bool MonsterLineOfSightToPlayer() const {
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        const Maze& maze = *world.maze;
        Tile mt = MonsterTile();
        Tile ct = CameraTile();
        if (!maze.IsOpen(mt.x, mt.y) || !maze.IsOpen(ct.x, ct.y)) return false;
        float dx = world.monsterPosition.x - world.playerPosition.x;
        float dz = world.monsterPosition.z - world.playerPosition.z;
        float visibleDistance = MonsterSightDistance();
        if (dx * dx + dz * dz > visibleDistance * visibleDistance) return false;
        return maze.LineClear(mt, ct);
    }

    bool MonsterVisualEncounterPlayer() const {
        if (monsterPreview_.active) return false;
        return MonsterLineOfSightToPlayer();
    }

    bool MonsterCuriosityActive() const {
        return gameWorld_.MonsterCuriosityActive();
    }

    float MonsterCuriosityAmount() const {
        return gameWorld_.MonsterCuriosityAmount();
    }

    bool ValidMonsterTile(Tile t) const {
        return RenderMazeView().IsOpen(t.x, t.y);
    }

    float MonsterAwarenessAmount() const {
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        const Maze& maze = *world.maze;
        float close = Clamp01((maze.TileAverage() * 7.0f - MonsterDistance()) / std::max(0.1f, maze.TileAverage() * 6.2f));
        float intent = std::max(monsterPresentation_.headChaseBlend, world.monsterChasePanic);
        if (world.monsterChasingVisible || gameWorld_.MonsterRecognizedForChase()) intent = std::max(intent, 1.0f);
        if (world.monsterHasLastKnownTarget) intent = std::max(intent, 0.78f);
        if (world.monsterHasSoundTarget) intent = std::max(intent, 0.56f);
        if (gameWorld_.MonsterChaseMemoryTimer() > 0.0f) intent = std::max(intent, 0.62f);
        return Clamp01(std::max(intent, close * 0.70f));
    }

    bool MonsterCanSeePlayer() const {
        if (MonsterIgnoresPlayer()) return false;
        return MonsterLineOfSightToPlayer();
    }
