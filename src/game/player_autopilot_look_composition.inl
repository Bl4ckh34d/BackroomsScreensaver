
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
        if (!panicActive && !softStopActive && pathTurnTargetWeight > 0.001f) {
            if (cameraRuntime_.turnLookBlend < 0.012f) {
                cameraRuntime_.turnLookYaw = pathTurnTargetYaw;
            } else {
                cameraRuntime_.turnLookYaw += AngleWrap(pathTurnTargetYaw - cameraRuntime_.turnLookYaw) * std::min(1.0f, dt * 6.2f);
            }
            float response = pathTurnTargetWeight > cameraRuntime_.turnLookBlend
                ? (2.35f + pathTurnTargetWeight * 2.80f)
                : 5.8f;
            cameraRuntime_.turnLookBlend += (pathTurnTargetWeight - cameraRuntime_.turnLookBlend) * std::min(1.0f, dt * response);
        } else {
            cameraRuntime_.turnLookBlend += (0.0f - cameraRuntime_.turnLookBlend) * std::min(1.0f, dt * 5.2f);
            if (cameraRuntime_.turnLookBlend < 0.001f) {
                cameraRuntime_.turnLookBlend = 0.0f;
                cameraRuntime_.turnLookYaw = desiredYaw;
            }
        }
        pathTurnWeight = cameraRuntime_.turnLookBlend;
        if (!panicActive && !softStopActive && pathTurnWeight > 0.001f) {
            desiredYaw += AngleWrap(cameraRuntime_.turnLookYaw - desiredYaw) * pathTurnWeight;
        }
        XMFLOAT3 exitFocusTarget{};
        float exitLookTargetWeight = (!panicActive && !softStopActive) ? ExitAttentionWeight(exitFocusTarget) : 0.0f;
        if (exitLookTargetWeight > 0.001f) {
            viewRuntime_.exitLookFocus = viewRuntime_.exitLookBlend <= 0.001f
                ? exitFocusTarget
                : Lerp3(viewRuntime_.exitLookFocus, exitFocusTarget, std::min(1.0f, dt * 5.4f));
        }
        float exitLookResponse = exitLookTargetWeight > viewRuntime_.exitLookBlend
            ? (2.20f + exitLookTargetWeight * 2.75f)
            : 4.8f;
        viewRuntime_.exitLookBlend += (exitLookTargetWeight - viewRuntime_.exitLookBlend) * std::min(1.0f, dt * exitLookResponse);
        if (viewRuntime_.exitLookBlend < 0.001f) viewRuntime_.exitLookBlend = 0.0f;

        float chaseLookBackWeight = threat ? ChaseLookBackWeight() : 0.0f;
        bool branchLookActive = !panicActive && cameraRuntime_.branchLookTimer > 0.0f && cameraRuntime_.branchLookDuration > 0.001f;
        float branchLookWeight = branchLookActive ? BranchLookWeight() : 0.0f;
        bool roomSurveyActive = !panicActive && cameraRuntime_.roomSurveyTimer > 0.0f && cameraRuntime_.roomSurveyDuration > 0.001f;
        float roomSurveyWeight = roomSurveyActive ? RoomSurveyWeight() : 0.0f;
        float stumbleAmount = 0.0f;
        if (viewRuntime_.stumbleTimer > 0.0f && viewRuntime_.stumbleDuration > 0.001f) {
            float t = 1.0f - viewRuntime_.stumbleTimer / viewRuntime_.stumbleDuration;
            stumbleAmount = std::sin(Clamp01(t) * kPi);
        }
        if (panicActive) {
            desiredYaw = gameWorld_.player.bodyYaw;
            if (threat && chaseLookBackWeight > 0.0f) {
                desiredYaw += AngleWrap(viewRuntime_.chaseLookBackYaw - desiredYaw) * (chaseLookBackWeight * 0.96f);
            }
            desiredYaw += viewRuntime_.stumbleYawOffset * stumbleAmount * (1.0f - chaseLookBackWeight);
        } else if (softStopActive) {
            if (ventReactionActive) {
                if (ventLookWeight > 0.001f) {
                    float ventTargetYaw = YawToPoint(viewRuntime_.ventReactionTarget);
                    float scanWeight = SmoothStep(0.10f, 1.0f, ventLookWeight);
                    float scanYaw = (std::sin(timeRuntime_.time * 7.9f + viewRuntime_.ventReactionScanSeed) * 0.042f +
                        std::sin(timeRuntime_.time * 13.7f + viewRuntime_.ventReactionScanSeed * 1.7f) * 0.018f) * scanWeight;
                    desiredYaw = gameWorld_.player.yaw + AngleWrap(ventTargetYaw - gameWorld_.player.yaw) * ventLookWeight + scanYaw;
                } else {
                    desiredYaw = gameWorld_.player.yaw + std::sin(timeRuntime_.time * 18.0f + viewRuntime_.ventReactionScanSeed) * 0.010f;
                }
            } else if (bloodFocusActive) desiredYaw = YawToPoint(scareRuntime_.bloodFocusTarget);
            else if (branchLookActive) {
                float lock = std::max(0.58f, branchLookWeight);
                desiredYaw = gameWorld_.player.yaw + AngleWrap(BranchLookTargetYaw() - gameWorld_.player.yaw) * lock;
            } else if (roomSurveyActive) desiredYaw = RoomSurveyYaw();
            else desiredYaw = gameWorld_.player.yaw;
        } else {
            Tile cameraTile = CameraTile();
            bool corridorLike = IsCorridorLike(cameraTile);
            if (branchLookActive &&
                ViewRayOpenDistance(cameraRuntime_.branchLookYaw, gameWorld_.maze.TileMinimum() * 2.2f) < gameWorld_.maze.TileMinimum() * 0.82f) {
                cameraRuntime_.branchLookTimer = 0.0f;
                branchLookActive = false;
                branchLookWeight = 0.0f;
            }
            float idleYaw = std::sin(timeRuntime_.time * 0.73f) * 0.045f + std::sin(timeRuntime_.time * 1.17f) * 0.025f;
            float idleScale = 1.0f;
            if (branchLookActive) idleScale *= 0.35f;
            if (roomSurveyActive) idleScale *= 0.50f;
            if (corridorLike) idleScale *= 0.38f;
            if (pathTurnWeight > 0.001f) idleScale *= Lerp(1.0f, 0.18f, Clamp01(pathTurnWeight / 0.52f));
            desiredYaw += idleYaw * idleScale;
            if (roomSurveyActive) {
                desiredYaw += AngleWrap(RoomSurveyYaw() - desiredYaw) * (roomSurveyWeight * 0.92f);
            } else if (branchLookActive) {
                desiredYaw += AngleWrap(BranchLookTargetYaw() - desiredYaw) * std::min(1.0f, branchLookWeight * 1.08f);
            }
            if (viewRuntime_.exitLookBlend > 0.001f) {
                desiredYaw += AngleWrap(YawToPoint(viewRuntime_.exitLookFocus) - desiredYaw) * viewRuntime_.exitLookBlend;
            }
        }
