static bool HandleConfigPreviewButtonDown(ConfigState* state, HWND hwnd, LPARAM lParam) {
    if (state && ConfigActiveTabIsMonsterPreview(state)) {
        POINT screenPoint = ConfigClientPointToScreen(hwnd, lParam);
        if (ConfigPointOverPreview(state, screenPoint)) {
            BeginConfigPreviewOrbit(state, hwnd, screenPoint);
            return true;
        }
    }
    return false;
}

static bool HandleConfigPreviewButtonUp(ConfigState* state, HWND hwnd) {
    if (state && state->previewOrbitDragging && GetCapture() == hwnd) {
        EndConfigPreviewOrbit(state, hwnd);
        return true;
    }
    return false;
}

static void HandleConfigCaptureChanged(ConfigState* state, HWND hwnd, LPARAM lParam) {
    if (state && state->previewOrbitDragging && reinterpret_cast<HWND>(lParam) != hwnd && GetCapture() != state->preview) {
        state->previewOrbitDragging = false;
    }
}

static bool HandleConfigPreviewMouseMove(ConfigState* state, HWND hwnd, LPARAM lParam) {
    if (state && state->previewOrbitDragging && ConfigActiveTabIsMonsterPreview(state) && GetCapture() == hwnd) {
        UpdateConfigPreviewOrbit(state, ConfigClientPointToScreen(hwnd, lParam));
        return true;
    }
    return false;
}
