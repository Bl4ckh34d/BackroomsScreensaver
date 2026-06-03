    void EnableInfiniteStaminaCheat() {
        sessionRuntime_.gameplaySettings.debugInfiniteStamina = true;
        settingsRuntime_.live.debugInfiniteStamina = true;
        gameWorld_.RefillPlayerStamina();
    }

    bool MonsterIgnoresPlayer() const {
        return !MonsterActiveForCurrentMode() || (settingsRuntime_.live.monsterIgnorePlayer && IsPlayableSimulationMode(sessionRuntime_.mode));
    }

    bool MonsterActiveForCurrentMode() const {
        if (monsterPreview_.active || gEffectDebugViewer) return true;
        if (sessionRuntime_.mode == RendererRuntimeMode::PlayableGame) {
            return gameWorld_.DeathActive() || gameWorld_.PlayableBossLevelRunning();
        }
        return true;
    }
