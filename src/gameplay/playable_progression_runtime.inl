    float UnitRandom() {
        return static_cast<float>(sessionRuntime_.rng() & 0xffffu) / 65535.0f;
    }

    bool SavedRunExists() const {
        std::error_code ec;
        return std::filesystem::exists(GameSavePath(), ec);
    }

    void DeleteSavedRun() const {
        std::error_code ec;
        std::filesystem::remove(GameSavePath(), ec);
    }

    void ApplyPlayableLevelSpec(const PlayableLevelSpec& spec) {
        float lampFlickerUnit = spec.NeedsLampFlickerRoll() ? UnitRandom() : 0.0f;
        spec.ApplyRuntimeSettings(
            settingsRuntime_.live,
            sessionRuntime_.gameplaySettings,
            lampFlickerUnit,
            gameWorld_.DarkLayerOneAppliesTo(spec.layer));
        if (!spec.bossEncounter) {
            gameWorld_.monster.ClearPursuitState();
            ResetMonsterPresentationState(true, false);
        }
    }

    bool SaveCurrentRunToFile() {
        if (sessionRuntime_.mode != RendererRuntimeMode::PlayableGame || !gameWorld_.CanSaveActiveRun()) return false;
        std::wostringstream out;
        out << L"Version=1\n";
        gameWorld_.WritePlayableRunFields(out);
        gameWorld_.WriteSavedRunFields(out);
        return WriteTextFile(GameSavePath(), out.str());
    }

    bool LoadSavedRunFromFile() {
        std::wstring text = ReadTextFile(GameSavePath());
        if (text.empty()) return false;
        auto values = ParseSavedRunKeyValues(text);
        if (SavedRunInt(values, L"Version", 0) != 1) return false;

        PlayableRunState::SavedRunRestoreState savedRun = PlayableRunState::ReadSavedRunFields(values);
        int savedSaveItemsSpawned = gameWorld_.RestoreSavedPlayableRunState(std::move(savedRun));
        PlayableLevelSpec spec = gameWorld_.CurrentPlayableLevel();
        ApplyPlayableLevelSpec(spec);
        if (gameWorld_.PlayableRunIsCustomGame()) gameWorld_.CurrentCustomSpec().ApplyScareSettings(settingsRuntime_.live);

        GameWorldSavedMazeRestoreState savedMaze = gameWorld_.ReadSavedMazeRestoreState(
            values,
            spec,
            MakeMazeLayoutSpec(settingsRuntime_.live));
        gameWorld_.RestoreSavedMazeGeometry(savedMaze);

        CreateMazeMaskTexture();
        ResetSimulation();
        gameWorld_.RestorePlayableSaveItemsSpawned(savedSaveItemsSpawned);
        CreateMazeMesh();
        SetupPersistentAudioEmitters();

        GameWorldSavedRuntimeRestoreState savedRuntime = gameWorld_.ReadSavedRuntimeRestoreState(values);
        gameWorld_.RestoreSavedRuntimeState(savedRuntime);
        sessionRuntime_.ConfigurePlayableManual();
        gameWorld_.progressionEnabled = true;
        ShowGameNotification(L"Saved run loaded", 3.8f);
        return true;
    }

    std::wstring FormatRunSeconds(float seconds) const {
        int whole = std::max(0, static_cast<int>(std::round(seconds)));
        int minutes = whole / 60;
        int secs = whole % 60;
        std::wostringstream s;
        s << minutes << L":";
        if (secs < 10) s << L"0";
        s << secs;
        return s.str();
    }

    void BeginPlayableLevel(int levelInLayer, bool showStartNotification = true) {
        PlayableLevelSpec level = gameWorld_.BuildLayerOneLevelSpec(levelInLayer, sessionRuntime_.rng);
        gameWorld_.BeginPlayableLevel(level);
        ApplyPlayableLevelSpec(gameWorld_.CurrentPlayableLevel());
        gameWorld_.ApplyMazeLayout(MakeMazeLayoutSpec(settingsRuntime_.live));
        RestartMaze();

        if (showStartNotification) {
            std::wostringstream notice;
            gameWorld_.WriteLevelStartNotice(notice);
            ShowGameNotification(notice.str(), 3.6f);
        }
    }

    void BeginPlayableRun() {
        gameWorld_.BeginLayerRun(menuRuntime_.darkLayerOneRun, sessionRuntime_.rng);
        gameWorld_.ResetMonsterKillCount();
        BeginPlayableLevel(1);
    }

    void BeginCustomPlayableRun(CustomGameSpec customSpec) {
        customSpec.Normalize();

        gameWorld_.BeginCustomRun(customSpec, sessionRuntime_.rng);

        PlayableLevelSpec level = CustomPlayableLevelSpec(customSpec);
        gameWorld_.BeginPlayableLevel(level);
        gameWorld_.ResetMonsterKillCount();

        ApplyPlayableLevelSpec(gameWorld_.CurrentPlayableLevel());
        customSpec.ApplyRuntimeSettings(settingsRuntime_.live, sessionRuntime_.gameplaySettings);
        if (BenchmarkDemoEnabled()) {
            ApplyBenchmarkDemoSettings(settingsRuntime_.live);
        }
        gameWorld_.ApplyMazeLayout(MakeMazeLayoutSpec(settingsRuntime_.live));
        RestartMaze();

        std::wostringstream notice;
        customSpec.WriteStartNotice(notice, gameWorld_.maze.w, gameWorld_.maze.h);
        ShowGameNotification(notice.str(), 3.8f);
    }

    void CompletePlayableLevel() {
        if (!gameWorld_.progressionEnabled) {
            gameWorld_.EndExitTransition();
            RestartMaze();
            return;
        }
        if (!gameWorld_.PlayableLevelRunning()) {
            gameWorld_.EndExitTransition();
            return;
        }

        DeleteSavedRun();
        PlayableLevelCompletionUpdate completion = gameWorld_.CompleteCurrentPlayableLevel();
        const PlayableLevelResult& result = completion.result;

        if (completion.finalRun) {
            gameWorld_.EndExitTransition();
            settingsRuntime_.live.monsterIgnorePlayer = true;
            cameraRuntime_.path.clear();
            cameraRuntime_.pathIndex = 0;
            std::wostringstream notice;
            notice << L"Layer complete\nTime " << FormatRunSeconds(gameWorld_.RunSeconds())
                   << L"   Score " << gameWorld_.TotalScore();
            if (completion.levelSecretTotal > 0) {
                notice << L"\nSecrets found " << completion.levelSecretsFound << L"/" << completion.levelSecretTotal;
            }
            notice << L"\nPress Esc for menu";
            ShowGameNotification(notice.str(), 3600.0f);
            return;
        }

        std::wostringstream notice;
        notice << L"Level " << result.levelInLayer << L" clear\n"
               << L"Time " << FormatRunSeconds(result.levelSeconds) << L"   Score +" << result.score
               << L"   Total " << gameWorld_.TotalScore();
        if (completion.levelSecretTotal > 0) {
            notice << L"\nSecrets found " << completion.levelSecretsFound << L"/" << completion.levelSecretTotal;
        }
        notice << L"\nPress Interact to continue";
        ShowGameNotification(notice.str(), 3600.0f);
        gameWorld_.EndExitTransition();
    }

    void ContinueAfterScoreScreen() {
        if (!gameWorld_.CanContinueAfterScoreScreen()) return;
        BeginPlayableLevel(gameWorld_.NextLevelAfterScoreScreen(), true);
    }

    void UpdatePlayableProgressionTimers(float dt) {
        if (sessionRuntime_.mode != RendererRuntimeMode::PlayableGame) return;
        gameWorld_.AdvancePlayableProgressionTimers(dt);
    }
