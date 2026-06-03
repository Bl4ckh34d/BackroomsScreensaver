        if (threat) {
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
