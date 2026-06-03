    bool EffectivePlayerCrouch() const {
        return sessionRuntime_.input.crouch || gameWorld_.PlayerTunnelCrouchLocked();
    }
