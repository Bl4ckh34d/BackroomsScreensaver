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
