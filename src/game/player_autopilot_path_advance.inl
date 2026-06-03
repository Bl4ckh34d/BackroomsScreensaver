        float bodyTargetYaw = dist > 0.001f ? std::atan2(dx, dz) : gameWorld_.player.bodyYaw;
        if (!freeRunMove && AdjacentTiles(movementTile, targetTileForMove)) {
            XMFLOAT3 currentCenter = gameWorld_.maze.WorldCenter(movementTile, gameWorld_.player.position.y);
            float alignTolerance = std::clamp(gameWorld_.maze.TileMinimum() * 0.055f, 0.045f, 0.11f);
            if (targetTileForMove.x != movementTile.x && std::abs(gameWorld_.player.position.z - currentCenter.z) > alignTolerance) {
                target = {gameWorld_.player.position.x, gameWorld_.player.position.y, currentCenter.z};
                dx = target.x - gameWorld_.player.position.x;
                dz = target.z - gameWorld_.player.position.z;
                dist = std::sqrt(dx * dx + dz * dz);
            } else if (targetTileForMove.y != movementTile.y && std::abs(gameWorld_.player.position.x - currentCenter.x) > alignTolerance) {
                target = {currentCenter.x, gameWorld_.player.position.y, gameWorld_.player.position.z};
                dx = target.x - gameWorld_.player.position.x;
                dz = target.z - gameWorld_.player.position.z;
                dist = std::sqrt(dx * dx + dz * dz);
            }
        }

        float pathAdvanceRadius = std::clamp(gameWorld_.maze.TileMinimum() * 0.045f, 0.045f, 0.085f);
        for (int advance = 0; !softStopActive && dist < pathAdvanceRadius && advance < 3; ++advance) {
            ++cameraRuntime_.pathIndex;
            if (CameraTile() == gameWorld_.maze.exit) {
                BeginExitTransition();
                return;
            }
            Tile cur = CameraTile();
            bool pauseStarted = false;
            if (!(cur == cameraRuntime_.lastTile)) {
                Tile previousTile = cameraRuntime_.lastTile;
                cameraRuntime_.previousTile = previousTile;
                cameraRuntime_.lastTile = cur;
                MarkVisited(cur);
                bool roomSurveySpot = IsRoomSurveySpot(cur);
                bool roomEntry = roomSurveySpot && !IsRoomSurveySpot(previousTile);
                Tile nextTarget = cameraRuntime_.pathIndex < cameraRuntime_.path.size() ? cameraRuntime_.path[cameraRuntime_.pathIndex] : cur;
                bool nextIsAdjacent = AdjacentTiles(cur, nextTarget);
                int approachX = cur.x - previousTile.x;
                int approachY = cur.y - previousTile.y;
                int exitX = nextTarget.x - cur.x;
                int exitY = nextTarget.y - cur.y;
                bool continuingStraight = nextIsAdjacent && exitX == approachX && exitY == approachY;
                bool turningOrChoosing = nextIsAdjacent && !continuingStraight;
                bool hasSideBranch = PathSideBranchCount(cur, previousTile, nextTarget) > 0;
                if (!panicActive && turningOrChoosing && hasSideBranch && IsTightTJunction(cur, previousTile) &&
                    RandRange(0.0f, 1.0f) < settingsRuntime_.live.junctionScanChance) {
                    pauseStarted = BeginJunctionScan(cur, previousTile);
                } else if (!panicActive && roomEntry && RandRange(0.0f, 1.0f) < settingsRuntime_.live.roomPauseChance) {
                    ChoosePath(true);
                    BeginRoomSurvey(cur, true);
                    pauseStarted = cameraRuntime_.roomSurveyTimer > 0.0f;
                } else if (!panicActive && roomEntry && RandRange(0.0f, 1.0f) < 0.72f) {
                    BeginRoomSurvey(cur, false);
                } else if (!panicActive && roomSurveySpot && !roomEntry && cameraRuntime_.roomSurveyCooldown <= 0.0f &&
                    RandRange(0.0f, 1.0f) < 0.24f) {
                    BeginRoomSurvey(cur, RandRange(0.0f, 1.0f) < 0.38f);
                    pauseStarted = cameraRuntime_.stopTimer > 0.0f && cameraRuntime_.roomSurveyTimer > 0.0f;
                } else if (!panicActive && hasSideBranch) {
                    bool hallwaySidePeek = continuingStraight && !roomSurveySpot;
                    bool decisionPeek = turningOrChoosing && !roomSurveySpot;
                    bool roomEdgePeek = roomSurveySpot && RandRange(0.0f, 1.0f) < 0.42f;
                    if (hallwaySidePeek || decisionPeek || roomEdgePeek) {
                        float chance = decisionPeek ? 0.88f : (hallwaySidePeek ? 0.74f : 0.46f);
                        if (RandRange(0.0f, 1.0f) < chance) {
                            bool allowPause = hallwaySidePeek
                                ? RandRange(0.0f, 1.0f) < 0.34f
                                : RandRange(0.0f, 1.0f) < 0.18f;
                            bool startedPeek = BeginBranchLook(cur, previousTile, nextTarget, allowPause, roomSurveySpot);
                            pauseStarted = startedPeek && cameraRuntime_.branchLookPaused;
                        }
                    }
                }
            }
            softStopActive = !panicActive && (cameraRuntime_.stopTimer > 0.0f || bloodFocusActive || ventReactionActive);
            if (pauseStarted || softStopActive) {
                if (pauseStarted && cameraRuntime_.junctionScanActive) {
                    return;
                }
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

        if (cameraRuntime_.pathIndex < cameraRuntime_.path.size()) {
            Tile bodyTile = freeRunMove && moveIndex < cameraRuntime_.path.size() ? cameraRuntime_.path[moveIndex] : cameraRuntime_.path[cameraRuntime_.pathIndex];
            XMFLOAT3 bodyTarget = gameWorld_.maze.WorldCenter(bodyTile, gameWorld_.player.position.y);
            float bdx = bodyTarget.x - gameWorld_.player.position.x;
            float bdz = bodyTarget.z - gameWorld_.player.position.z;
            if (bdx * bdx + bdz * bdz > 0.0001f) {
                bodyTargetYaw = std::atan2(bdx, bdz);
            }
        }
        gameWorld_.player.bodyYaw += AngleWrap(bodyTargetYaw - gameWorld_.player.bodyYaw) * std::min(1.0f, dt * 5.2f);
        UpdatePropLook(dt, panicActive);
