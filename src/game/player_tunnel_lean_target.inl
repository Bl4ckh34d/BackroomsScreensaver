    void UpdateTunnelLeanSideTarget(Tile tile, const XMFLOAT3& intendedMoveDir, bool hasMoveDir) {
        gameWorld_.SetPlayerTunnelLeanSideTarget(TunnelLeanSideFromApproach(tile, intendedMoveDir, hasMoveDir));
    }
