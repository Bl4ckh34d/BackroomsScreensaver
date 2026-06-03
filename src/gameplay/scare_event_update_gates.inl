        scareRuntime_.scareCooldown = std::max(0.0f, scareRuntime_.scareCooldown - dt);
        scareRuntime_.fleshFlickerTimer = std::max(0.0f, scareRuntime_.fleshFlickerTimer - dt);
        scareRuntime_.bloodWorldFlickerTimer = std::max(0.0f, scareRuntime_.bloodWorldFlickerTimer - dt);
        float scareFrequency = JumpscareFrequency();
        float scareScale = ScareCooldownScale();
        auto customScareAllowed = [&](int index) {
            PlayableCustomScareGate gate = gameWorld_.CustomScareGateFor(index);
            if (!gate.allowed) return false;
            return !gate.requiresRoll || RandRange(0.0f, 1.0f) <= gate.chance;
        };
        if (scareFrequency <= 0.001f) {
            scareRuntime_.fleshFlickerTimer = 0.0f;
            scareRuntime_.fleshFlickerCooldown = 1000000.0f;
            scareRuntime_.bloodWorldFlickerTimer = 0.0f;
            scareRuntime_.bloodWorldFlickerCooldown = 1000000.0f;
            return;
        }
        if (IsThreatVisible() || ChasePanicActive()) {
            scareRuntime_.scareCooldown = std::max(scareRuntime_.scareCooldown, 0.80f);
            viewRuntime_.ventReactionTimer = 0.0f;
            return;
        }
        if (sessionRuntime_.mode == RendererRuntimeMode::PlayableGame && sessionRuntime_.input.crouch) {
            scareRuntime_.scareCooldown = std::max(scareRuntime_.scareCooldown, 0.35f);
            viewRuntime_.ventReactionTimer = 0.0f;
            return;
        }
