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
