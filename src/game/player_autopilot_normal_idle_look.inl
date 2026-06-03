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
