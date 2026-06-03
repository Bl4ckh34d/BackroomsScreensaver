        if (panicActive) {
            cameraRuntime_.stopTimer = 0.0f;
            cameraRuntime_.headScanTimer = 0.0f;
            cameraRuntime_.headScanDuration = 0.0f;
            cameraRuntime_.lookBack = false;
            cameraRuntime_.junctionScanActive = false;
            viewRuntime_.propLookTimer = 0.0f;
            cameraRuntime_.branchLookTimer = 0.0f;
            cameraRuntime_.roomSurveyTimer = 0.0f;
            cameraRuntime_.branchLookPaused = false;

            viewRuntime_.chaseLookBackCooldown = std::max(0.0f, viewRuntime_.chaseLookBackCooldown - dt);
            viewRuntime_.chaseLookBackTimer = std::max(0.0f, viewRuntime_.chaseLookBackTimer - dt);
            if (threat && viewRuntime_.chaseLookBackTimer <= 0.0f && viewRuntime_.chaseLookBackCooldown <= 0.0f && MonsterDistance() > 1.55f) {
                float urgency = Clamp01((7.8f - MonsterDistance()) / 6.4f);
                float elapsedPressure = Clamp01(viewRuntime_.secondsSinceLookBack / 9.0f);
                float chance = dt * (0.22f + urgency * 1.55f + elapsedPressure * 0.80f);
                if (RandRange(0.0f, 1.0f) < chance) {
                    BeginChaseLookBack(urgency);
                }
            }
            if (threat) UpdateChaseLookBackTarget(dt);

            viewRuntime_.stumbleTimer = std::max(0.0f, viewRuntime_.stumbleTimer - dt);
            if (threat && viewRuntime_.stumbleTimer <= 0.0f && viewRuntime_.chaseLookBackTimer <= 0.0f && RandRange(0.0f, 1.0f) < dt * 0.14f) {
                viewRuntime_.stumbleDuration = RandRange(0.18f, 0.34f);
                viewRuntime_.stumbleTimer = viewRuntime_.stumbleDuration;
                viewRuntime_.stumbleYawOffset = RandRange(-0.20f, 0.20f);
            }
        } else if (roomSurveySpaceNow && !bloodFocusActive && !ventReactionActive && cameraRuntime_.stopTimer <= 0.0f &&
            viewRuntime_.propLookTimer <= 0.0f && cameraRuntime_.branchLookTimer <= 0.0f && cameraRuntime_.roomSurveyTimer <= 0.0f &&
            timeRuntime_.time > cameraRuntime_.nextLookBackTime) {
            float elapsedPressure = Clamp01(viewRuntime_.secondsSinceLookBack / std::max(2.0f, settingsRuntime_.live.lookBackMaxSeconds));
            float proximityPressure = Clamp01((10.0f - MonsterDistance()) / 8.0f) * 0.35f;
            float chance = dt * (0.08f + elapsedPressure * 0.62f + proximityPressure);
            if (RandRange(0.0f, 1.0f) < chance) {
                cameraRuntime_.stopTimer = RandRange(1.05f, 1.85f);
                cameraRuntime_.headScanDuration = cameraRuntime_.stopTimer;
                cameraRuntime_.headScanTimer = cameraRuntime_.stopTimer;
                cameraRuntime_.headScanCenter = gameWorld_.player.bodyYaw + RandRange(-0.16f, 0.16f);
                cameraRuntime_.lookBack = true;
                cameraRuntime_.branchLookTimer = 0.0f;
                cameraRuntime_.roomSurveyTimer = 0.0f;
                cameraRuntime_.nextLookBackTime = timeRuntime_.time + RandRange(settingsRuntime_.live.lookBackMinSeconds, settingsRuntime_.live.lookBackMaxSeconds);
                viewRuntime_.secondsSinceLookBack = 0.0f;
            }
        }

        if (!panicActive && !ventReactionActive && cameraRuntime_.headScanTimer > 0.0f) {
            cameraRuntime_.headScanTimer = std::max(0.0f, cameraRuntime_.headScanTimer - dt);
            float t = cameraRuntime_.headScanDuration > 0.001f ? 1.0f - cameraRuntime_.headScanTimer / cameraRuntime_.headScanDuration : 1.0f;
            float scanAngle = settingsRuntime_.live.scanAngleDegrees * kPi / 180.0f;
            float desired = cameraRuntime_.headScanCenter;
            float scanSpeed = 4.6f;
            Tile scanTile = CameraTile();
            bool corridorScanBlocked = !IsRoomSurveySpot(scanTile) && !cameraRuntime_.junctionScanActive;
            if (corridorScanBlocked) {
                cameraRuntime_.headScanTimer = 0.0f;
                cameraRuntime_.headScanDuration = 0.0f;
                cameraRuntime_.lookBack = false;
                cameraRuntime_.stopTimer = 0.0f;
            }
            if (!corridorScanBlocked && cameraRuntime_.junctionScanActive && cameraRuntime_.junctionScanCount > 0) {
                float segment = Clamp01(t) * static_cast<float>(cameraRuntime_.junctionScanCount);
                int scanIndex = std::min(cameraRuntime_.junctionScanCount - 1, static_cast<int>(segment));
                float localT = segment - static_cast<float>(scanIndex);
                float settle = SmoothStep(0.10f, 0.42f, localT) * (1.0f - SmoothStep(0.82f, 1.0f, localT));
                float branchYaw = cameraRuntime_.junctionScanYaws[static_cast<size_t>(scanIndex)];
                desired = branchYaw + std::sin(timeRuntime_.time * 5.3f + scanIndex) * 0.035f * settle;
                scanSpeed = 7.8f;
            } else if (!corridorScanBlocked) {
                float sweep = std::sin(t * kPi * 2.0f);
                float rawDesired = cameraRuntime_.lookBack
                    ? cameraRuntime_.headScanCenter + kPi + sweep * 0.22f
                    : cameraRuntime_.headScanCenter + sweep * scanAngle;
                if (cameraRuntime_.lookBack) {
                    desired = rawDesired;
                    scanSpeed = 7.4f;
                } else {
                    desired = rawDesired;
                }
            }
            if (!corridorScanBlocked) {
                gameWorld_.player.yaw += AngleWrap(desired - gameWorld_.player.yaw) * std::min(1.0f, dt * scanSpeed);
                gameWorld_.player.pitch += (-0.035f - gameWorld_.player.pitch) * std::min(1.0f, dt * 3.0f);
            }
        }

        if (cameraRuntime_.stopTimer > 0.0f) {
            cameraRuntime_.stopTimer = std::max(0.0f, cameraRuntime_.stopTimer - dt);
            float idleY = 1.47f + std::sin(timeRuntime_.time * 2.1f) * 0.008f;
            gameWorld_.player.position.y += (idleY - gameWorld_.player.position.y) * std::min(1.0f, dt * 3.0f);
            if (cameraRuntime_.stopTimer <= 0.0f) {
                completedJunctionScan = cameraRuntime_.junctionScanActive;
                cameraRuntime_.lookBack = false;
                cameraRuntime_.junctionScanActive = false;
            }
        }

        if (ventReactionActive) {
            if (ventBackAwayWeight > 0.001f) {
                float backSpeed = std::max(settingsRuntime_.live.walkSpeed * 1.10f, gameWorld_.maze.TileMinimum() * 0.42f);
                float nudge = backSpeed * ventBackAwayWeight * dt;
                Tile cur = CameraTile();
                MovePlayerThroughCollision(viewRuntime_.ventReactionAway.x * nudge, viewRuntime_.ventReactionAway.z * nudge,
                    nudge, backSpeed, cur);
            }
        }

        if (!PlayerCollisionFootprintOpen(gameWorld_.player.position.x, gameWorld_.player.position.z)) {
            RecoverPlayerCollisionFootprint();
            return;
        }
        if (!panicActive && cameraRuntime_.stopTimer > 0.0f && (cameraRuntime_.lookBack || cameraRuntime_.junctionScanActive)) {
            return;
        }
