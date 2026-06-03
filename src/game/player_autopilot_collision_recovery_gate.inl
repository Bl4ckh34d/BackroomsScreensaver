        if (!PlayerCollisionFootprintOpen(gameWorld_.player.position.x, gameWorld_.player.position.z)) {
            RecoverPlayerCollisionFootprint();
            return;
        }
        if (!panicActive && cameraRuntime_.stopTimer > 0.0f && (cameraRuntime_.lookBack || cameraRuntime_.junctionScanActive)) {
            return;
        }
