struct GameMenuButtonSpec {
    int id;
    const wchar_t* label;
};

std::vector<GameMenuButtonSpec> ActiveGameMenuButtons() {
    bool canResume = gApp && gApp->gameRunStarted && !gApp->gameDebugActive;
    bool canResumeSaved = std::filesystem::exists(GameSavePath());
    std::vector<GameMenuButtonSpec> buttons;
    buttons.reserve(5);
    if (canResume) buttons.push_back({kGameResumeCurrentRunId, L"Resume"});
    if (canResumeSaved) buttons.push_back({kGameResumeSavedRunId, L"Resume Saved Run"});
    buttons.push_back({kGameSinglePlayerId, L"New Game"});
    buttons.push_back({kGameCustomGameId, L"Custom Game"});
    buttons.push_back({kGameSettingsId, L"Settings"});
    return buttons;
}
