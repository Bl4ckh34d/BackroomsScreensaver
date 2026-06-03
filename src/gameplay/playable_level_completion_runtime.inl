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
            ShowScoreNotification(notice.str(), 3600.0f);
            return;
        }

        std::wostringstream notice;
        notice << L"Level " << result.levelInLayer << L" clear\n"
               << L"Time " << FormatRunSeconds(result.levelSeconds) << L"   Score +" << result.score
               << L"   Total " << gameWorld_.TotalScore();
        if (completion.levelSecretTotal > 0) {
            notice << L"\nSecrets found " << completion.levelSecretsFound << L"/" << completion.levelSecretTotal;
        }
        notice << L"\nPress any key to continue";
        ShowScoreNotification(notice.str(), 3600.0f);
    }
