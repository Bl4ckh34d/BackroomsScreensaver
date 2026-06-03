    // Runtime mode, session, save, notification, and live game settings APIs.

    void SetRuntimeMode(RendererRuntimeMode mode) {
        if (mode == RendererRuntimeMode::PlayableGame) {
            sessionRuntime_.ConfigurePlayableManual();
            gameWorld_.progressionEnabled = true;
        } else if (mode == RendererRuntimeMode::ScreensaverAutopilot) {
            sessionRuntime_.ConfigureScreensaverAutopilot();
            gameWorld_.progressionEnabled = false;
        } else {
            sessionRuntime_.mode = mode;
        }
    }

    RendererRuntimeMode RuntimeMode() const {
        return sessionRuntime_.mode;
    }

    void SetGameInput(const GameInputSnapshot& input) {
        sessionRuntime_.input = input;
    }

    bool LoadSavedGameRun() {
        return LoadSavedRunFromFile();
    }

    bool SavedGameRunExists() const {
        return SavedRunExists();
    }

    bool PlayableRunFinished() const {
        return sessionRuntime_.mode == RendererRuntimeMode::PlayableGame && gameWorld_.PlayableRunFinished();
    }

    void DeleteSavedGameRun() {
        DeleteSavedRun();
    }

    void StartGameSession(const GameSessionSpec& spec) {
        gEffectDebugViewer = false;
        gBloodDebugEveryWall = false;
        sessionRuntime_.ConfigureFromSpec(spec);
        gameWorld_.progressionEnabled = spec.progressionEnabled &&
            spec.runtimeMode == RendererRuntimeMode::PlayableGame;
        EnsureFullSceneAssets();
        settingsRuntime_.live = sessionRuntime_.gameplaySettings;
        menuRuntime_.startTransitionActive = false;
        menuRuntime_.startTransitionComplete = false;
        menuRuntime_.startTransitionFromCustomView = false;
        menuRuntime_.startTransitionTimer = 0.0f;
        menuRuntime_.startTransitionFade = 0.0f;
        menuRuntime_.customViewTarget = false;
        menuRuntime_.customViewActive = false;
        mapOverlayRuntime_.cachedVerts.clear();
        mapOverlayRuntime_.nextUpdateTime = 0.0f;
        gameWorld_.ResetPlayerInputLatches(true);
        sessionRuntime_.ClearInput();

        if (sessionRuntime_.mode == RendererRuntimeMode::PlayableGame) {
            if (spec.customGame) {
                BeginCustomPlayableRun(spec.customSpec);
            } else {
                BeginPlayableRun();
            }
            return;
        }

        gameWorld_.ResetPlayableRun();
        gameWorld_.ResetMonsterKillCount();
        RestartMaze();
    }

    void StartScreensaverSession() {
        StartGameSession(GameSessionSpec::ScreensaverAutopilot());
    }

    void RestartCustomGameRun(const CustomGameSpec& customSpec) {
        StartGameSession(GameSessionSpec::CustomPlayableRun(customSpec));
    }

    void ShowGameNotification(const std::wstring& text, float durationSeconds = 4.2f) {
        hudNotification_.text = text;
        hudNotification_.startTime = timeRuntime_.time;
        hudNotification_.duration = std::max(0.25f, durationSeconds);
        hudNotification_.textureDirty = true;
    }

    void ApplyGameSettings(const Settings& settings) {
        auto applyLive = [&](Settings& target) {
            target.gameFullscreen = settings.gameFullscreen;
            target.gameResolutionWidth = settings.gameResolutionWidth;
            target.gameResolutionHeight = settings.gameResolutionHeight;
            target.allowWarpFallback = settings.allowWarpFallback;
            target.mapOverlay = settings.mapOverlay;
            target.debugAiMapOverlay = settings.debugAiMapOverlay;
            target.debugInfiniteStamina = settings.debugInfiniteStamina;
            target.debugInvincible = settings.debugInvincible;
            target.monsterIgnorePlayer = settings.monsterIgnorePlayer;
            target.exposure = settings.exposure;
            target.bloomAmount = settings.bloomAmount;
            target.motionBlurAmount = settings.motionBlurAmount;
            target.airParticleDensity = settings.airParticleDensity;
            target.mouseSensitivity = settings.mouseSensitivity;
            target.invertMouseY = settings.invertMouseY;
            target.gameKeyBindings = settings.gameKeyBindings;
            target.audioMuted = settings.audioMuted;
            target.audioMasterVolume = settings.audioMasterVolume;
            target.audioEffectsVolume = settings.audioEffectsVolume;
            target.audioAmbienceVolume = settings.audioAmbienceVolume;
            target.audioMonsterVolume = settings.audioMonsterVolume;
        };
        applyLive(sessionRuntime_.gameplaySettings);
        applyLive(settingsRuntime_.live);
        audioRuntime_.engine.ApplySettings(MakeAudioEngineSettings(settingsRuntime_.live));
    }

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

    void RestartGameRun() {
        StartGameSession(GameSessionSpec::PlayableRun());
    }
