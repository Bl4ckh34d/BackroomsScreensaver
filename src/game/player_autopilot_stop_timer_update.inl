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
