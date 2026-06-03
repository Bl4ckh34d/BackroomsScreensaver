            if (point.waterLiquid) continue;
            if (horizontalDist > tileAvg * 3.45f) continue;
            float visibleAge = timeRuntime_.time - point.activationTime;
            float minVisibleAge = point.requireFacing
                ? 1.10f
                : (target.y > settingsRuntime_.live.wallHeightMeters * 0.55f ? 0.92f : 0.76f);
            if (point.focusTaken || visibleAge < std::max(point.focusDelaySeconds, minVisibleAge)) continue;
            bool focusAllowed = scareRuntime_.bloodFocusReactionsTaken == 0 ||
                (scareRuntime_.bloodFocusReactionsTaken == 1 && scareRuntime_.bloodFocusReactionCooldown <= 0.0f);
            if (!focusAllowed) continue;

            scareRuntime_.activeBloodScareIndex = static_cast<int>(i);
            scareRuntime_.bloodScareActiveUntil = std::max(scareRuntime_.bloodScareActiveUntil, timeRuntime_.time + 150.0f);
            point.focusTaken = true;
            ++scareRuntime_.bloodFocusReactionsTaken;
            scareRuntime_.bloodFocusReactionCooldown = scareRuntime_.bloodFocusReactionsTaken == 1
                ? RandRange(5.0f, 15.0f)
                : 1000000.0f;
            scareRuntime_.bloodFocusDuration = RandRange(1.45f, 2.25f);
            scareRuntime_.bloodFocusTimer = scareRuntime_.bloodFocusDuration;
            scareRuntime_.bloodFocusTarget = {
                target.x,
                std::clamp(target.y, 0.16f, settingsRuntime_.live.wallHeightMeters - 0.05f),
                target.z
            };
            cameraRuntime_.stopTimer = 0.0f;
            cameraRuntime_.headScanTimer = 0.0f;
            cameraRuntime_.lookBack = false;
            cameraRuntime_.junctionScanActive = false;
            viewRuntime_.propLookTimer = 0.0f;
