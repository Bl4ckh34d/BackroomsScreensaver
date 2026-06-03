        auto tryAxisMove = [&](float sx, float sz) {
            float axisDistance = std::sqrt(sx * sx + sz * sz);
            if (axisDistance <= 0.0001f) return false;
            GameWorldRenderSnapshot axisWorld = gameWorld_.BuildRenderSnapshot();
            float ax = axisWorld.playerPosition.x + sx;
            float az = axisWorld.playerPosition.z + sz;
            bool axisOpen = freeRun
                ? PlayerCollisionSegmentOpenThroughOpen(axisWorld.playerPosition.x, axisWorld.playerPosition.z, ax, az, false)
                : PlayerCollisionSegmentOpen(axisWorld.playerPosition.x, axisWorld.playerPosition.z, ax, az, startTile, allowedTarget);
            if (!axisOpen) return false;
            gameWorld_.MovePlayerHorizontalAndAdvanceStep(
                ax,
                az,
                axisDistance,
                speed,
                CurrentPlayerMovementTuning());
            return true;
        };

        if (std::abs(stepX) > 0.0001f && std::abs(stepZ) > 0.0001f) {
            bool tryZFirst = std::abs(stepZ) >= std::abs(stepX);
            if (tryZFirst) {
                if (tryAxisMove(0.0f, stepZ)) return true;
                if (tryAxisMove(stepX, 0.0f)) return true;
            } else {
                if (tryAxisMove(stepX, 0.0f)) return true;
                if (tryAxisMove(0.0f, stepZ)) return true;
            }
        }
