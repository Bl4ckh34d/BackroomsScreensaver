    int ScoreCompletedPlayableLevel(float levelSeconds, bool bossEncounter) const;
    int ScoreCurrentPlayableLevel() const;
    bool CanAdvancePlayableProgression() const;
    PlayableLevelCompletionUpdate CompleteCurrentPlayableLevel();
    bool CanContinueAfterScoreScreen() const;
    int NextLevelAfterScoreScreen() const;
    bool AdvancePlayableProgressionTimers(float dt);
