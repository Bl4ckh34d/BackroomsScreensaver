    void RestoreFlashlightState(bool flashlightEnabled);
    void RestoreInteractLatch(bool interactPressedLastFrame);
    void ResetPlayerInputLatches(bool flashlightEnabled);
    void RefillPlayerStamina();
    void RestorePlayerFullVitals();
    void ResetPlayerForSession(float initialSmoothedMoveSpeed);
    void SetPlayerCameraPose(const XMFLOAT3& position, float yaw, float bodyYaw, float pitch);
    void SetPlayerHorizontalPosition(float x, float z);
    void AdvancePlayerStepPhase(float metersMoved, float speedMetersPerSecond, const PlayerMovementTuning& tuning);
    void MovePlayerHorizontalAndAdvanceStep(float x, float z, float metersMoved, float speedMetersPerSecond, const PlayerMovementTuning& tuning);
    PlayerManualControlResult UpdatePlayerManualControl(
        const PlayerManualControlInput& input,
        const PlayerManualControlServices& services);

    void RaisePlayerRunPressure(float minRunIntensity, float minRunEffort, float minSmoothedMoveSpeed);
    void SetPlayerSmoothedMoveSpeed(float speed);
    Tile StartTile() const;
    XMFLOAT3 StartWorldCenter(float y) const;
    XMFLOAT3 ExitWorldCenter(float y) const;
    float MazeTileMinimum() const;
    bool DeathActive() const;
    size_t TileIndex(Tile t) const;
    uint16_t VisitCount(Tile t) const;
    bool HasVisitedMapTiles() const;
    void MarkVisited(Tile t);
    bool PlayerTunnelCrouchLocked() const;
    float PlayerTunnelLeanSideTarget() const;
    float PlayerTunnelPostureHoldTimer() const;
    void ResetPlayerTunnelPosture();
    void SetPlayerTunnelCrouchLocked(bool locked);
    void HoldPlayerTunnelCrouch(float holdSeconds);
    void SetPlayerTunnelLeanSideTarget(float side);
    void SetPlayerTunnelLeanTarget(bool active);
    void AdvancePlayerTunnelPostureTimer(float dt);
    void AdvancePlayerTunnelCameraLean(float dt);
