    void WriteSavedRunFields(std::wostream& out) const;
    GameWorldSavedMazeRestoreState ReadSavedMazeRestoreState(
        const SavedRunKeyValues& values,
        const PlayableLevelSpec& spec,
        const MazeLayoutSpec& fallbackLayout) const;
    void RestoreSavedMazeGeometry(const GameWorldSavedMazeRestoreState& saved);
    GameWorldSavedRuntimeRestoreState ReadSavedRuntimeRestoreState(const SavedRunKeyValues& values) const;
    void RestoreSavedRuntimeState(const GameWorldSavedRuntimeRestoreState& saved);
