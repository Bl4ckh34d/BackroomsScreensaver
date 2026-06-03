    void UpdateTunnelCrouchLock(float dt, const XMFLOAT3& intendedMoveDir, bool hasMoveDir) {
        dt = std::max(0.0f, dt);
        if (sessionRuntime_.mode != RendererRuntimeMode::PlayableGame) {
            gameWorld_.ResetPlayerTunnelPosture();
            return;
        }

        gameWorld_.AdvancePlayerTunnelPostureTimer(dt);
        Tile tile = CameraTile();
        bool inTunnel = IsTunnelTile(tile);
        Tile approachTile{};
        bool approachingTunnel = sessionRuntime_.input.crouch && ProbeTunnelAhead(intendedMoveDir, hasMoveDir, approachTile);
        Tile vicinityTile{};
        bool nearTunnel = TunnelImmediateVicinity(vicinityTile);
        bool tunnelMouthLock = nearTunnel && (sessionRuntime_.input.crouch || gameWorld_.PlayerTunnelCrouchLocked());

        if (inTunnel) {
            gameWorld_.HoldPlayerTunnelCrouch(0.58f);
        } else if (tunnelMouthLock) {
            gameWorld_.SetPlayerTunnelCrouchLocked(true);
            UpdateTunnelLeanSideTarget(vicinityTile, intendedMoveDir, hasMoveDir);
        } else if (approachingTunnel) {
            UpdateTunnelLeanSideTarget(approachTile, intendedMoveDir, hasMoveDir);
            if (gameWorld_.PlayerTunnelPostureHoldTimer() <= 0.0f && !sessionRuntime_.input.crouch) gameWorld_.SetPlayerTunnelCrouchLocked(false);
        } else if (!sessionRuntime_.input.crouch) {
            gameWorld_.SetPlayerTunnelCrouchLocked(false);
        }

        gameWorld_.SetPlayerTunnelLeanTarget(inTunnel || tunnelMouthLock || approachingTunnel || gameWorld_.PlayerTunnelPostureHoldTimer() > 0.0f);
    }
