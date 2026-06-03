static void HandleConfigNotify(ConfigState* state, LPARAM lParam) {
    if (state && reinterpret_cast<NMHDR*>(lParam)->idFrom == kConfigTabId &&
        reinterpret_cast<NMHDR*>(lParam)->code == TCN_SELCHANGE) {
        ShowConfigTab(state, TabCtrl_GetCurSel(state->tab));
    }
}

static bool HandleConfigPreviewTimer(ConfigState* state, WPARAM wParam) {
    if (state && wParam == kConfigPreviewTimerId) {
        if (state->previewPending && GetTickCount64() >= state->previewApplyAt) {
            RestartConfigPreview(state);
        }
        if (state->previewRenderer) state->previewRenderer->Tick();
        return true;
    }
    return false;
}
