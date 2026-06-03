        if (ventReactionActive) {
            if (ventBackAwayWeight > 0.001f) {
                float backSpeed = std::max(settingsRuntime_.live.walkSpeed * 1.10f, gameWorld_.maze.TileMinimum() * 0.42f);
                float nudge = backSpeed * ventBackAwayWeight * dt;
                Tile cur = CameraTile();
                MovePlayerThroughCollision(viewRuntime_.ventReactionAway.x * nudge, viewRuntime_.ventReactionAway.z * nudge,
                    nudge, backSpeed, cur);
            }
        }
