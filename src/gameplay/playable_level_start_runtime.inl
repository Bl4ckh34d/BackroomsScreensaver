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
        if (AutoplayBenchmarkEnabled() && AutoplayBenchmarkBossLevel() == level.levelInLayer) {
            level.bossEncounter = true;
            level.bossEncounterChance = 1.0f;
        }
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
        bool darkLayerOne = AutoplayBenchmarkEnabled() ? false : menuRuntime_.darkLayerOneRun;
        gameWorld_.BeginLayerRun(darkLayerOne, sessionRuntime_.rng);
        gameWorld_.ResetMonsterKillCount();
        int startLevel = AutoplayBenchmarkEnabled() ? AutoplayBenchmarkStartLevel() : 1;
        BeginPlayableLevel(startLevel);
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
