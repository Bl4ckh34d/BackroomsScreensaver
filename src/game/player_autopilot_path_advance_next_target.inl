            if (cameraRuntime_.pathIndex >= cameraRuntime_.path.size()) ChoosePath();
                if (cameraRuntime_.pathIndex >= cameraRuntime_.path.size() || !ActivePathValidForMode(CameraTile(), panicActive && OpenAreaAllowsFreeRun(CameraTile()))) return;
                target = gameWorld_.maze.WorldCenter(cameraRuntime_.path[cameraRuntime_.pathIndex], gameWorld_.player.position.y);
                dx = target.x - gameWorld_.player.position.x;
                dz = target.z - gameWorld_.player.position.z;
                dist = std::sqrt(dx * dx + dz * dz);
                break;
            }
            if (cameraRuntime_.pathIndex >= cameraRuntime_.path.size()) {
                ChoosePath(true);
                if (cameraRuntime_.pathIndex >= cameraRuntime_.path.size() || !ActivePathValidForMode(CameraTile(), panicActive && OpenAreaAllowsFreeRun(CameraTile()))) return;
            }
            target = gameWorld_.maze.WorldCenter(cameraRuntime_.path[cameraRuntime_.pathIndex], gameWorld_.player.position.y);
            dx = target.x - gameWorld_.player.position.x;
            dz = target.z - gameWorld_.player.position.z;
            dist = std::sqrt(dx * dx + dz * dz);
        }
