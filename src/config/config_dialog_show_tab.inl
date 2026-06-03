void ShowConfigTab(ConfigState* state, int tab) {
    if (!state) return;
    tab = std::clamp(tab, 0, std::max(0, state->tabCount - 1));
    state->activeTab = tab;
    if (!ConfigActiveTabIsMonsterPreview(state) && state->previewOrbitDragging) {
        EndConfigPreviewOrbit(state, GetCapture());
    }
    if (state->note && tab < static_cast<int>(state->tabNotes.size())) SetWindowTextW(state->note, state->tabNotes[static_cast<size_t>(tab)].c_str());
    ApplyConfigScroll(state);
    state->previewPending = false;
    UpdateConfigPreviewHint(state);
    if (!state->previewRenderer) {
        SetConfigPreviewStatus(state, L"Click Update preview to render current settings.");
    } else {
        SetConfigPreviewStatus(state, L"");
    }
}
