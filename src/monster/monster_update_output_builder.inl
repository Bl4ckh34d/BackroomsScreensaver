        float dt = input.dt;
        auto makeOutput = [&](bool seesPlayer, bool heardPlayer, bool hasGoal, bool moved) {
            MonsterUpdateOutput output{};
            output.seesPlayer = seesPlayer;
            output.heardPlayer = heardPlayer;
            output.hasGoal = hasGoal;
            output.moved = moved;
            output.distanceToPlayer = MonsterDistance();
            bool killPlayer = !settingsRuntime_.live.debugInvincible && !input.ignorePlayer &&
                output.distanceToPlayer < settingsRuntime_.live.monsterKillDistance &&
                gameWorld_.maze.LineClear(CameraTile(), MonsterTile());
            if (killPlayer) output.AddGameplayEvent(MonsterGameplayEventKind::KillPlayer);
            return output;
        };
