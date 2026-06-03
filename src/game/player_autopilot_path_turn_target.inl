        Tile cameraTileForLook = CameraTile();
        float pathTurnWeight = 0.0f;
        float desiredYaw = bodyTargetYaw;
        float pathTurnTargetWeight = 0.0f;
        float pathTurnTargetYaw = desiredYaw;
        if (cameraRuntime_.pathIndex < cameraRuntime_.path.size()) {
            bool roomLook = OpenAreaAllowsFreeRun(cameraTileForLook);
            float lookAheadTiles = roomLook
                ? std::clamp(settingsRuntime_.live.roomLookAheadTiles, 0.0f, 10.0f)
                : std::clamp(settingsRuntime_.live.turnLookAheadTiles, 0.0f, 8.0f);
            XMFLOAT3 lookTarget = PathLookAheadPoint(lookAheadTiles);
            float lx = lookTarget.x - gameWorld_.player.position.x;
            float lz = lookTarget.z - gameWorld_.player.position.z;
            if (lx * lx + lz * lz > 0.0001f) {
                desiredYaw = std::atan2(lx, lz);
            }

            size_t turnIndex = cameraRuntime_.pathIndex;
            if (cameraRuntime_.path[turnIndex] == cameraTileForLook && turnIndex + 1 < cameraRuntime_.path.size()) {
                ++turnIndex;
            }
            if (turnIndex + 1 < cameraRuntime_.path.size()) {
                Tile approach = AdjacentTiles(cameraTileForLook, cameraRuntime_.path[turnIndex])
                    ? cameraTileForLook
                    : (turnIndex > 0 ? cameraRuntime_.path[turnIndex - 1] : cameraTileForLook);
                Tile turn = cameraRuntime_.path[turnIndex];
                Tile after = cameraRuntime_.path[turnIndex + 1];
                if (AdjacentTiles(approach, turn) && AdjacentTiles(turn, after)) {
                    int inX = turn.x - approach.x;
                    int inY = turn.y - approach.y;
                    int outX = after.x - turn.x;
                    int outY = after.y - turn.y;
                    bool changedDirection = inX != outX || inY != outY;
                    bool reversed = outX == -inX && outY == -inY;
                    if (changedDirection && !reversed) {
                        XMFLOAT3 turnCenter = gameWorld_.maze.WorldCenter(turn, gameWorld_.player.position.y);
                        XMFLOAT3 afterCenter = gameWorld_.maze.WorldCenter(after, gameWorld_.player.position.y);
                        float distToTurn = std::sqrt((turnCenter.x - gameWorld_.player.position.x) * (turnCenter.x - gameWorld_.player.position.x) +
                            (turnCenter.z - gameWorld_.player.position.z) * (turnCenter.z - gameWorld_.player.position.z));
                        float tile = std::max(gameWorld_.maze.TileMinimum(), 0.1f);
                        float turnLeadTiles = std::clamp(settingsRuntime_.live.turnLookAheadTiles, 0.0f, 8.0f);
                        float turnLeadStart = tile * std::clamp(0.86f + turnLeadTiles * 0.42f, 0.86f, 4.25f);
                        float turnLead = SmoothStep(turnLeadStart, tile * 0.18f, distToTurn);
                        turnLead *= turnLead;
                        pathTurnTargetWeight = turnLead * 0.68f;
                        float outgoingYaw = std::atan2(afterCenter.x - turnCenter.x, afterCenter.z - turnCenter.z);
                        pathTurnTargetYaw = outgoingYaw;
                    }
                }
            }
        }
