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
