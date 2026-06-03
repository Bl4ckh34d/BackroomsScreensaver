        scareRuntime_.bloodFocusReactionCooldown = std::max(0.0f, scareRuntime_.bloodFocusReactionCooldown - dt);
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        if (world.deathActive || world.exitTransitionActive || scareRuntime_.bloodScarePoints.empty()) return;
        if (IsThreatVisible() || ChasePanicActive()) {
            scareRuntime_.bloodFocusTimer = 0.0f;
            scareRuntime_.activeBloodScareIndex = -1;
            return;
        }
        if (sessionRuntime_.mode == RendererRuntimeMode::PlayableGame && sessionRuntime_.input.crouch) {
            scareRuntime_.bloodFocusTimer = 0.0f;
            scareRuntime_.activeBloodScareIndex = -1;
            return;
        }
