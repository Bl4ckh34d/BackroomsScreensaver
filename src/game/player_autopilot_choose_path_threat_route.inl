        if (threat) {
            if (AutoplayBenchmarkEnabled() &&
                !AutoplayBenchmarkExploreLevel() &&
                gameWorld_.progressionEnabled &&
                gameWorld_.PlayableLevelRunning()) {
                auto exitPath = maze.Path(cur, maze.exit);
                if (!exitPath.empty()) {
                    viewRuntime_.exitSpotted = true;
                    cameraRuntime_.path = std::move(exitPath);
                    cameraRuntime_.pathIndex = cameraRuntime_.path.size() > 1 ? 1 : 0;
                    return;
                }
            }

            if (viewRuntime_.exitSpotted && ExitRouteNotBlockedByMonster()) {
                auto exitPath = maze.Path(cur, maze.exit);
                if (!exitPath.empty() && !ThreatPathMovesTowardMonster(exitPath, monsterTile)) {
                    cameraRuntime_.path = std::move(exitPath);
                    cameraRuntime_.pathIndex = cameraRuntime_.path.size() > 1 ? 1 : 0;
                    return;
                }
            }

            cameraRuntime_.path = BuildThreatEscapePath(cur, monsterTile);
            if (cameraRuntime_.path.empty()) {
                ForceImmediateFleeStep(cur, monsterTile);
                return;
            }
