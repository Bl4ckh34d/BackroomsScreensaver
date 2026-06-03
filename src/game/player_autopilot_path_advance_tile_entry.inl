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
