    PlayableLevelCompletionUpdate CompleteCurrentLevel(int score) {
        PlayableLevelCompletionUpdate update{};
        levelRunning = false;

        update.result.layer = layer;
        update.result.levelInLayer = levelInLayer;
        update.result.levelSeconds = levelSeconds;
        update.result.runSeconds = runSeconds;
        update.result.bossEncounter = currentLevel.bossEncounter;
        update.result.score = score;

        totalScore += update.result.score;
        lastResult = update.result;
        completed.push_back(update.result);
        completedLevels = static_cast<int>(completed.size());
        scoreScreenActive = true;

        ReconcileLayerPagesCollected();

        update.levelSecretTotal = LayerPageCountForLevel(update.result.levelInLayer);
        update.levelSecretsFound = update.levelSecretTotal > 0
            ? std::clamp(LayerPagesCollectedForLevel(update.result.levelInLayer), 0, update.levelSecretTotal)
            : 0;

        update.finalRun = customGame || levelInLayer >= kLevelsPerLayer;
        if (update.finalRun) {
            runFinished = true;
            active = false;
            scoreScreenFinal = true;
        }
        return update;
    }

    bool CanContinueAfterScoreScreen() const {
        return scoreScreenActive && !scoreScreenFinal;
    }

    int NextLevelAfterScoreScreen() const {
        return std::clamp(levelInLayer + 1, 1, kLevelsPerLayer);
    }

    void AdvanceRunningTimers(float dt) {
        if (!active || !levelRunning) return;
        float step = std::max(0.0f, dt);
        runSeconds += step;
        levelSeconds += step;
    }
