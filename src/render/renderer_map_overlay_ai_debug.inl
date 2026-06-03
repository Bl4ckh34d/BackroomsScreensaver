        bool drawAiDebug = settingsRuntime_.live.debugAiMapOverlay ||
            sessionRuntime_.mapOverlayStyle == GameSessionMapOverlayStyle::AiDebug;
        if (drawAiDebug) {
            bool alertPath = world.monsterHasSoundTarget || world.monsterHasLastKnownTarget || world.monsterChasingVisible;
            XMFLOAT4 pathColor = alertPath ? XMFLOAT4{1.0f, 0.12f, 0.04f, 0.72f} : XMFLOAT4{1.0f, 0.56f, 0.10f, 0.52f};
            size_t pathRemaining = monsterPath.size() > world.monsterPathIndex ? monsterPath.size() - world.monsterPathIndex : 0;
            size_t pathStep = std::max<size_t>(1, (pathRemaining + 95) / 96);
            for (size_t i = world.monsterPathIndex; i < monsterPath.size(); i += pathStep) {
                float t = monsterPath.size() > world.monsterPathIndex
                    ? static_cast<float>(i - world.monsterPathIndex) / static_cast<float>(std::max<size_t>(1, monsterPath.size() - world.monsterPathIndex))
                    : 0.0f;
                XMFLOAT4 color = pathColor;
                color.w *= Lerp(1.0f, 0.46f, Clamp01(t));
                pushTile(monsterPath[i], color, 0.26f);
            }
            if (world.monsterHeardPlayerNow) markTile(CameraTile(), {1.0f, 0.0f, 0.0f, 1.0f}, 2.35f);
            if (world.monsterHasSoundTarget) markTile(world.monsterSoundTile, {1.0f, 0.02f, 0.02f, 0.96f}, 2.05f);
            if (world.monsterHasLastKnownTarget) markTile(world.monsterLastKnownTile, {1.0f, 0.88f, 0.16f, 0.90f}, 1.85f);
            if (ValidMonsterTile(world.monsterGoal)) markTile(world.monsterGoal, {1.0f, 0.38f, 0.02f, 0.78f}, 1.62f);
            if (ValidMonsterTile(world.monsterRoamTile) && !world.monsterHasSoundTarget && !world.monsterHasLastKnownTarget) {
                markTile(world.monsterRoamTile, {0.82f, 0.68f, 0.42f, 0.56f}, 1.42f);
            }
            for (Tile occupied : MonsterBodyOccupiedTiles()) {
                markTile(occupied, {0.88f, 0.02f, 0.04f, 0.58f}, 1.34f);
            }
            markTile(MonsterTile(), {1.0f, 0.02f, 0.02f, 0.78f}, 1.55f);
        }
