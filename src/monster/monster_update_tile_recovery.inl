        gameWorld_.monster.repathTimer -= dt;
        Tile mt = MonsterTile();
        Tile ct = CameraTile();
        gameWorld_.monster.heardPlayerNow = false;
        if (!gameWorld_.maze.IsOpen(mt.x, mt.y)) {
            if (!RecoverMonsterToNearestOpenTile(mt)) {
                gameWorld_.monster.position = gameWorld_.maze.WorldCenter(gameWorld_.maze.exit, 0.0f);
                ResetMonsterPresentationState(true, true, false);
                RecordMonsterTrailPoint(gameWorld_.monster.position);
                ClearMonsterPath();
            }
            UpdateMonsterHeadAnimation(dt, false);
            return makeOutput(false, false, false, false);
        }
