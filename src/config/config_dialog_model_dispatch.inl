void BuildConfigModel(ConfigState* state) {
    if (!state) return;
    state->fieldDefs.clear();
    state->tabLabels.clear();
    state->tabNotes.clear();
    if (state->mode == ConfigDialogMode::Game) {
        BuildGameConfigModel(state);
    } else if (state->mode == ConfigDialogMode::Debug) {
        BuildDebugConfigModel(state);
    } else {
        BuildFullConfigModel(state);
    }
    state->tabCount = static_cast<int>(std::min<size_t>(state->tabLabels.size(), kConfigMaxTabCount));
}
